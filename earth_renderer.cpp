#include "earth_renderer.h"
#include <QVector2D>
#include <QVector3D>
#include <QtMath>
#include <QCoreApplication>

EarthRenderer::EarthRenderer(float radius)
    : Renderer()
    , earthTextureTiles(nullptr)
    , heightMapTiles(nullptr)
    , normalMapTiles(nullptr)
    , indexBuffer(QOpenGLBuffer::IndexBuffer)
    , radius(radius)
    , vertexCount(0)
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

    initShaders();
    initTextures();
    initGeometry();
}

void EarthRenderer::initTextures()
{
    QString buildDir = QCoreApplication::applicationDirPath();

    earthTextureTiles = new TileTextureManager(buildDir + "/textures/earth.jpg", RINGS, SEGMENTS);
    heightMapTiles = new TileTextureManager(buildDir + "/textures/earth_height.png", RINGS, SEGMENTS);
    normalMapTiles = new TileTextureManager(buildDir + "/textures/earth_normal.png", RINGS, SEGMENTS);

    earthTextureTiles->initialize();
    heightMapTiles->initialize();
    normalMapTiles->initialize();
}

void EarthRenderer::createSphere()
{
    vertices.clear();
    indices.clear();

    // Для отладки
    qDebug() << "Creating sphere with RINGS:" << RINGS << "SEGMENTS:" << SEGMENTS;

    for (int ring = 0; ring <= RINGS; ++ring) {
        float phi = ring * M_PI / RINGS;
        // Текстурная координата V (от 0 до 1)
        float v = static_cast<float>(ring) / RINGS;

        for (int segment = 0; segment <= SEGMENTS; ++segment) {
            float theta = segment * 2.0f * M_PI / SEGMENTS;
            // Текстурная координата U (от 0 до 1)
            float u = static_cast<float>(segment) / SEGMENTS;

            // Вычисляем позицию
            float x = sin(phi) * cos(theta) * radius;
            float y = cos(phi) * radius;
            float z = sin(phi) * sin(theta) * radius;

            // Создаем вершину
            Vertex vertex;
            vertex.position = QVector3D(x, y, z);
            vertex.normal = vertex.position.normalized();
            vertex.texCoord = QVector2D(u, v);
            vertex.segmentIndex = QVector2D(ring, segment);

            vertices.append(vertex);

            // Отладочный вывод для первых нескольких вершин
            if (ring <= 1 && segment <= 1) {
                qDebug() << "Vertex" << ring << segment
                         << "pos:" << vertex.position
                         << "uv:" << vertex.texCoord
                         << "segmentIndex:" << vertex.segmentIndex;
            }
        }
    }

    // Генерация индексов
    for (int ring = 0; ring < RINGS; ++ring) {
        for (int segment = 0; segment < SEGMENTS; ++segment) {
            int current = ring * (SEGMENTS + 1) + segment;
            int next = current + SEGMENTS + 1;

            // Первый треугольник
            indices.append(current);
            indices.append(next);
            indices.append(current + 1);

            // Второй треугольник
            indices.append(current + 1);
            indices.append(next);
            indices.append(next + 1);
        }
    }

    vertexCount = indices.size();

    // Загружаем данные в буферы
    vbo.create();
    vbo.bind();
    vbo.allocate(vertices.constData(), vertices.size() * sizeof(Vertex));

    indexBuffer.create();
    indexBuffer.bind();
    indexBuffer.allocate(indices.constData(), indices.size() * sizeof(GLuint));

    // Настраиваем атрибуты вершин
    program.enableAttributeArray(0);  // position
    program.setAttributeBuffer(0, GL_FLOAT, offsetof(Vertex, position), 3, sizeof(Vertex));

    program.enableAttributeArray(1);  // texCoord
    program.setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex, texCoord), 2, sizeof(Vertex));

    program.enableAttributeArray(2);  // normal
    program.setAttributeBuffer(2, GL_FLOAT, offsetof(Vertex, normal), 3, sizeof(Vertex));

    program.enableAttributeArray(3);  // segmentIndex
    program.setAttributeBuffer(3, GL_FLOAT, offsetof(Vertex, segmentIndex), 2, sizeof(Vertex));
}

void EarthRenderer::updateVisibleTiles(const QMatrix4x4& viewProjection, const QMatrix4x4& model)
{
    earthTextureTiles->updateVisibleTiles(viewProjection, model);
    heightMapTiles->updateVisibleTiles(viewProjection, model);
    normalMapTiles->updateVisibleTiles(viewProjection, model);
}

void EarthRenderer::render(const QMatrix4x4& projection, const QMatrix4x4& view, const QMatrix4x4& model)
{
    if (!program.isLinked()) return;

    QMatrix4x4 viewProjection = projection * view;
    updateVisibleTiles(viewProjection, model);

    program.bind();
    vao.bind();

    // Матрицы
    QMatrix4x4 mvp = projection * view * model;
    program.setUniformValue("mvp", mvp);
    program.setUniformValue("model", model);
    program.setUniformValue("normalMatrix", model.normalMatrix());
    program.setUniformValue("gridThickness", 0.001f);
    program.setUniformValue("gridColor", QVector4D(1.0f, 1.0f, 0.0f, 0.5f));
    program.setUniformValue("numSegments", SEGMENTS);
    program.setUniformValue("numRings", RINGS);

    // Настройка состояния OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Для каждого видимого сегмента
    for (int ring = 0; ring < RINGS; ++ring) {
        for (int segment = 0; segment < SEGMENTS; ++segment) {
            if (earthTextureTiles->isTileVisible(ring, segment)) {
                // Устанавливаем текущий сегмент
                program.setUniformValue("currentRing", ring);
                program.setUniformValue("currentSegment", segment);

                // Привязываем текстуры для текущего сегмента
                glActiveTexture(GL_TEXTURE0);
                earthTextureTiles->bindTileForSegment(ring, segment);
                program.setUniformValue("earthTexture", 0);

                glActiveTexture(GL_TEXTURE1);
                heightMapTiles->bindTileForSegment(ring, segment);
                program.setUniformValue("heightMap", 1);

                glActiveTexture(GL_TEXTURE2);
                normalMapTiles->bindTileForSegment(ring, segment);
                program.setUniformValue("normalMap", 2);

                // Отрисовываем всю геометрию
                // Шейдер сам отфильтрует ненужные фрагменты
                glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, nullptr);
            }
        }
    }

    // Возвращаем состояние OpenGL
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    vao.release();
    program.release();
}

void EarthRenderer::initShaders()
{
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/earth_vertex.glsl")) {
        qDebug() << "Failed to compile vertex shader";
        return;
    }

    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/earth_fragment.glsl")) {
        qDebug() << "Failed to compile fragment shader";
        return;
    }

    if (!program.link()) {
        qDebug() << "Failed to link shader program";
        return;
    }
}

void EarthRenderer::initGeometry()
{
    vao.create();
    vao.bind();
    createSphere();
    vao.release();
}

GLint EarthRenderer::getMaxTextureSize()
{
    GLint maxSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    return maxSize;
}
