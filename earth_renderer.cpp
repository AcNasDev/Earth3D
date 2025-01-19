#include "earth_renderer.h"
#include <QImage>
#include <QtMath>
#include <QCoreApplication>
#include <QImageReader>
#include <QDebug>

EarthRenderer::EarthRenderer(float radius)
    : Renderer()
    , earthTextureTiles(nullptr)
    , heightMapTiles(nullptr)
    , normalMapTiles(nullptr)
    , indexBuffer(QOpenGLBuffer::IndexBuffer)
    , radius(radius)
    , vertexCount(0)
    , displacementScale(DEFAULT_DISPLACEMENT_SCALE)
    , texturesInitialized(false)
{
}

EarthRenderer::~EarthRenderer()
{
    delete earthTextureTiles;
    delete heightMapTiles;
    delete normalMapTiles;

    if (indexBuffer.isCreated()) {
        indexBuffer.destroy();
    }
}

void EarthRenderer::initialize()
{
    if (!init()) {
        qDebug() << "Failed to initialize OpenGL functions for EarthRenderer";
        return;
    }

    // Проверяем максимальный поддерживаемый размер текстур
    GLint maxTextureSize = getMaxTextureSize();
    qDebug() << "Maximum supported texture size:" << maxTextureSize;

    initShaders();
    initTextures();
    initGeometry();

    earthTextureTiles->loadAllTiles();
    heightMapTiles->loadAllTiles();
    normalMapTiles->loadAllTiles();
}

void EarthRenderer::initShaders()
{
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/earth_vertex.glsl")) {
        qDebug() << "Failed to compile earth vertex shader";
    }

    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/earth_fragment.glsl")) {
        qDebug() << "Failed to compile earth fragment shader";
    }

    if (!program.link()) {
        qDebug() << "Failed to link earth shader program";
    }
}

int EarthRenderer::calculateOptimalTileSize(int width, int height)
{
    // Получаем максимальный поддерживаемый размер текстуры
    GLint maxTextureSize = getMaxTextureSize();

    // Начинаем с размера 2048
    int tileSize = 2048;

    // Проверяем, сколько тайлов получится
    int tilesX = (width + tileSize - 1) / tileSize;
    int tilesY = (height + tileSize - 1) / tileSize;

    // Если количество тайлов превышает 8x8 (64 тайла), увеличиваем размер тайла
    while (tilesX * tilesY > 64 && tileSize < maxTextureSize) {
        tileSize *= 2;
        tilesX = (width + tileSize - 1) / tileSize;
        tilesY = (height + tileSize - 1) / tileSize;
    }

    return tileSize;
}

void EarthRenderer::initTextures()
{
    QString buildDir = QCoreApplication::applicationDirPath();

    // Загружаем изображения для определения их размеров
    QImage earthTexture(buildDir + "/textures/earth.jpg");
    QImage heightMap(buildDir + "/textures/earth_height.png");
    QImage normalMap(buildDir + "/textures/earth_normal.png");

    // Находим максимальные размеры
    int maxWidth = qMax(qMax(earthTexture.width(), heightMap.width()), normalMap.width());
    int maxHeight = qMax(qMax(earthTexture.height(), heightMap.height()), normalMap.height());

    // Рассчитываем оптимальный размер тайла
    int tileSize = calculateOptimalTileSize(maxWidth, maxHeight);

    qDebug() << "Texture sizes:"
             << "\nEarth texture:" << earthTexture.size()
             << "\nHeight map:" << heightMap.size()
             << "\nNormal map:" << normalMap.size()
             << "\nOptimal tile size:" << tileSize;

    // Создаем менеджеры тайлов с одинаковым размером тайла
    earthTextureTiles = new TileTextureManager(buildDir + "/textures/earth.jpg", tileSize);
    heightMapTiles = new TileTextureManager(buildDir + "/textures/earth_height.png", tileSize);
    normalMapTiles = new TileTextureManager(buildDir + "/textures/earth_normal.png", tileSize);

    // Инициализируем менеджеры
    earthTextureTiles->initialize();
    heightMapTiles->initialize();
    normalMapTiles->initialize();

    // Проверяем количество тайлов
    int earthTilesCount = earthTextureTiles->getTilesX() * earthTextureTiles->getTilesY();
    int heightTilesCount = heightMapTiles->getTilesX() * heightMapTiles->getTilesY();
    int normalTilesCount = normalMapTiles->getTilesX() * normalMapTiles->getTilesY();

    qDebug() << "Tiles count:"
             << "\nEarth texture:" << earthTilesCount
             << "\nHeight map:" << heightTilesCount
             << "\nNormal map:" << normalTilesCount;

    texturesInitialized = true;
}

void EarthRenderer::initGeometry()
{
    vao.create();
    vao.bind();

    vbo.create();
    indexBuffer.create();

    createSphere(RINGS, SEGMENTS);

    vao.release();
}

void EarthRenderer::createSphere(int rings, int segments)
{
    QVector<GLfloat> vertices;
    QVector<GLuint> indices;

    // Генерация вершин сферы
    for (int ring = 0; ring <= rings; ++ring) {
        float phi = ring * M_PI / rings;
        for (int segment = 0; segment <= segments; ++segment) {
            float theta = segment * 2.0f * M_PI / segments;

            // Позиция
            float x = sin(phi) * cos(theta) * radius;
            float y = cos(phi) * radius;
            float z = sin(phi) * sin(theta) * radius;

            // Текстурные координаты
            // Инвертируем только U координату, V оставляем как есть
            float u = 1.0f - static_cast<float>(segment) / segments;
            float v = static_cast<float>(ring) / rings;  // Убираем инверсию V

            // Нормаль (направлена наружу от центра сферы)
            float nx = sin(phi) * cos(theta);
            float ny = cos(phi);
            float nz = sin(phi) * sin(theta);

            // Добавляем вершину
            vertices << x << y << z;      // позиция
            vertices << u << v;           // текстурные координаты
            vertices << nx << ny << nz;   // нормаль
        }
    }

    // Генерация индексов
    for (int ring = 0; ring < rings; ++ring) {
        for (int segment = 0; segment < segments; ++segment) {
            GLuint current = ring * (segments + 1) + segment;
            GLuint next = current + segments + 1;

            indices << current << current + 1 << next;
            indices << next << current + 1 << next + 1;
        }
    }

    vertexCount = indices.size();

    // Создаем и привязываем VAO
    vao.create();
    vao.bind();

    // Создаем, привязываем и заполняем VBO
    vbo.create();
    vbo.bind();
    vbo.allocate(vertices.constData(), vertices.size() * sizeof(GLfloat));

    // Создаем, привязываем и заполняем IBO
    indexBuffer.create();
    indexBuffer.bind();
    indexBuffer.allocate(indices.constData(), indices.size() * sizeof(GLuint));

    // Настраиваем атрибуты вершин
    // Позиция
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), nullptr);

    // Текстурные координаты
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                          reinterpret_cast<void*>(3 * sizeof(GLfloat)));

    // Нормаль
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                          reinterpret_cast<void*>(5 * sizeof(GLfloat)));

    vao.release();
    indexBuffer.release();
    vbo.release();
}

GLint EarthRenderer::getMaxTextureSize()
{
    GLint maxSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    return maxSize;
}

void EarthRenderer::render(const QMatrix4x4& projection, const QMatrix4x4& view, const QMatrix4x4& model)
{
    if (!texturesInitialized) return;

    program.bind();
    vao.bind();

    // Матрицы
    QMatrix4x4 mvp = projection * view * model;
    program.setUniformValue("mvp", mvp);
    program.setUniformValue("model", model);
    program.setUniformValue("normalMatrix", model.normalMatrix());

    // Передаем информацию о тайлах
    program.setUniformValue("tilesX", earthTextureTiles->getTilesX());
    program.setUniformValue("tilesY", earthTextureTiles->getTilesY());

    // Привязываем текстуры
    glActiveTexture(GL_TEXTURE0);
    earthTextureTiles->bindAllTiles();
    program.setUniformValue("earthTexture", 0);

    glActiveTexture(GL_TEXTURE1);
    heightMapTiles->bindAllTiles();
    program.setUniformValue("heightMap", 1);

    glActiveTexture(GL_TEXTURE2);
    normalMapTiles->bindAllTiles();
    program.setUniformValue("normalMap", 2);

    // Настройка отсечения граней
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);    // Отсекаем задние грани
    glFrontFace(GL_CCW);    // Передние грани определяются против часовой стрелки

    // Отрисовка
    glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, nullptr);

    // Возвращаем состояние OpenGL
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    vao.release();
    program.release();
}
