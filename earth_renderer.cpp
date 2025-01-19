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

void EarthRenderer::initTextures()
{
    QString buildDir = QCoreApplication::applicationDirPath();
    int tileSize = 2048; // Оптимальный размер тайла

    // Создаем менеджеры тайлов
    earthTextureTiles = new TileTextureManager(buildDir + "/textures/earth.jpg", tileSize);
    heightMapTiles = new TileTextureManager(buildDir + "/textures/earth_height.png", tileSize);
    normalMapTiles = new TileTextureManager(buildDir + "/textures/earth_normal.png", tileSize);

    // Инициализируем менеджеры
    earthTextureTiles->initialize();
    heightMapTiles->initialize();
    normalMapTiles->initialize();

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
            float x = sin(phi) * cos(theta);
            float y = cos(phi);
            float z = sin(phi) * sin(theta);

            vertices << x << y << z;

            // Текстурные координаты - важно генерировать их правильно
            float u = 1.0f - static_cast<float>(segment) / segments;
            float v = 1.0f - static_cast<float>(ring) / rings;
            vertices << u << v;

            // Нормали
            vertices << x << y << z;

            // Отладка
            qDebug() << "Vertex:" << x << y << z << "TexCoord:" << u << v;
        }
    }

    // Генерация индексов
    for (int ring = 0; ring < rings; ++ring) {
        for (int segment = 0; segment < segments; ++segment) {
            GLuint first = ring * (segments + 1) + segment;
            GLuint second = first + segments + 1;

            indices << first << first + 1 << second;
            indices << second << first + 1 << second + 1;
        }
    }

    vertexCount = indices.size();

    // Загружаем данные в буферы
    vbo.bind();
    vbo.allocate(vertices.constData(), vertices.size() * sizeof(GLfloat));

    indexBuffer.bind();
    indexBuffer.allocate(indices.constData(), indices.size() * sizeof(GLuint));

    // Настраиваем атрибуты вершин
    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), nullptr);

    glEnableVertexAttribArray(1); // texcoord
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                          reinterpret_cast<void*>(3 * sizeof(GLfloat)));

    glEnableVertexAttribArray(2); // normal
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                          reinterpret_cast<void*>(5 * sizeof(GLfloat)));
}

GLint EarthRenderer::getMaxTextureSize()
{
    GLint maxSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    return maxSize;
}

QVector2D EarthRenderer::calculateTextureCoordinate(const QVector3D& cameraPos)
{
    // Преобразуем позицию камеры в сферические координаты
    float longitude = atan2(cameraPos.z(), cameraPos.x());
    float latitude = asin(cameraPos.y() / cameraPos.length());

    // Нормализуем долготу и широту в диапазон [0,1]
    // longitude: от -PI до PI -> [0,1]
    // latitude: от -PI/2 до PI/2 -> [0,1]
    float u = (longitude + M_PI) / (2.0f * M_PI);
    float v = (latitude + M_PI_2) / M_PI;

    qDebug() << "Camera spherical coords - longitude:" << longitude
             << "latitude:" << latitude
             << "UV coords:" << u << v;

    return QVector2D(u, v);
}

void EarthRenderer::render(const QMatrix4x4& projection, const QMatrix4x4& view, const QMatrix4x4& model)
{
    if (!texturesInitialized) {
        return;
    }

    program.bind();
    vao.bind();

    // Создаем матрицу для Земли
    QMatrix4x4 earthMatrix = model;
    earthMatrix.scale(radius);

    // Устанавливаем униформы для шейдеров
    program.setUniformValue("mvp", projection * view * earthMatrix);
    program.setUniformValue("model", earthMatrix);
    program.setUniformValue("normalMatrix", earthMatrix.normalMatrix());

    // Получаем позицию камеры
    QVector3D cameraPos = view.inverted().column(3).toVector3D();
    QVector2D texCoord = calculateTextureCoordinate(cameraPos);
    program.setUniformValue("viewPos", cameraPos);

    // Устанавливаем масштаб смещения для рельефа
    program.setUniformValue("displacementScale", displacementScale);

    // Привязываем текстуры и обновляем тайлы
    // Текстурный юнит 0 для цветовой текстуры
    glActiveTexture(GL_TEXTURE0);
    earthTextureTiles->bindTileForCoordinate(texCoord);
    program.setUniformValue("earthTexture", 0);

    // Текстурный юнит 1 для карты высот
    glActiveTexture(GL_TEXTURE1);
    heightMapTiles->bindTileForCoordinate(texCoord);
    program.setUniformValue("heightMap", 1);

    // Текстурный юнит 2 для карты нормалей
    glActiveTexture(GL_TEXTURE2);
    normalMapTiles->bindTileForCoordinate(texCoord);
    program.setUniformValue("normalMap", 2);

    // Передаем информацию о текущем тайле в шейдер
    QVector4D tileInfo = heightMapTiles->getCurrentTileInfo(texCoord);
    program.setUniformValue("tileInfo", tileInfo);

    // Рендерим сферу
    glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, nullptr);

    // Очищаем состояние
    vao.release();
    program.release();
}
