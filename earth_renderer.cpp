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
    if (vbo.isCreated())
        vbo.destroy();
    if (ibo.isCreated())
        ibo.destroy();
    if (vao.isCreated())
        vao.destroy();
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

QVector3D EarthRenderer::sphericalToCartesian(float radius, float phi, float theta)
{
    float x = radius * sin(phi) * cos(theta);
    float y = radius * cos(phi);
    float z = radius * sin(phi) * sin(theta);
    return QVector3D(x, y, z);
}

void EarthRenderer::createSphere()
{
    vertices.clear();
    indices.clear();

    // Для каждого кольца (кроме полюсов)
    for (int ring = 0; ring < RINGS; ++ring) {
        float phi1 = M_PI * float(ring) / RINGS;        // Начало кольца
        float phi2 = M_PI * float(ring + 1) / RINGS;    // Конец кольца

        // Для каждого сегмента в кольце
        for (int segment = 0; segment < SEGMENTS; ++segment) {
            float theta1 = 2.0f * M_PI * float(segment) / SEGMENTS;       // Начало сегмента
            float theta2 = 2.0f * M_PI * float(segment + 1) / SEGMENTS;   // Конец сегмента

            // Создаем четырехугольник (два треугольника)
            // Вершины четырехугольника
            QVector3D v1 = sphericalToCartesian(radius, phi1, theta1);
            QVector3D v2 = sphericalToCartesian(radius, phi1, theta2);
            QVector3D v3 = sphericalToCartesian(radius, phi2, theta2);
            QVector3D v4 = sphericalToCartesian(radius, phi2, theta1);

            // UV координаты для тайла
            QVector2D uv1(float(segment) / SEGMENTS, float(ring) / RINGS);
            QVector2D uv2(float(segment + 1) / SEGMENTS, float(ring) / RINGS);
            QVector2D uv3(float(segment + 1) / SEGMENTS, float(ring + 1) / RINGS);
            QVector2D uv4(float(segment) / SEGMENTS, float(ring + 1) / RINGS);

            // Индекс текущего тайла
            QVector2D tileIndex(ring, segment);

            // Добавляем вершины
            int baseIndex = vertices.size();

            // Вершина 1
            Vertex vertex1;
            vertex1.position = v1;
            vertex1.texCoord = uv1;
            vertex1.normal = v1.normalized();
            vertex1.segmentIndex = tileIndex;
            vertices.append(vertex1);

            // Вершина 2
            Vertex vertex2;
            vertex2.position = v2;
            vertex2.texCoord = uv2;
            vertex2.normal = v2.normalized();
            vertex2.segmentIndex = tileIndex;
            vertices.append(vertex2);

            // Вершина 3
            Vertex vertex3;
            vertex3.position = v3;
            vertex3.texCoord = uv3;
            vertex3.normal = v3.normalized();
            vertex3.segmentIndex = tileIndex;
            vertices.append(vertex3);

            // Вершина 4
            Vertex vertex4;
            vertex4.position = v4;
            vertex4.texCoord = uv4;
            vertex4.normal = v4.normalized();
            vertex4.segmentIndex = tileIndex;
            vertices.append(vertex4);

            // Добавляем индексы для двух треугольников
            indices.append(baseIndex);      // v1
            indices.append(baseIndex + 1);  // v2
            indices.append(baseIndex + 2);  // v3

            indices.append(baseIndex);      // v1
            indices.append(baseIndex + 2);  // v3
            indices.append(baseIndex + 3);  // v4
        }
    }

    // Обновляем буферы
    updateBuffers();
}

void EarthRenderer::updateBuffers()
{
    // Создаем и настраиваем VAO если он еще не создан
    if (!vao.isCreated()) {
        vao.create();
    }
    vao.bind();

    // Буфер вершин
    if (!vbo.isCreated()) {
        vbo.create();
    }
    vbo.bind();
    vbo.allocate(vertices.data(), vertices.size() * sizeof(Vertex));

    // Буфер индексов
    if (!ibo.isCreated()) {
        ibo.create();
    }
    ibo.bind();
    ibo.allocate(indices.data(), indices.size() * sizeof(GLuint));

    // Настраиваем атрибуты вершин
    // Позиция
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, position)));

    // Текстурные координаты
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, texCoord)));

    // Нормаль
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, normal)));

    // Индекс сегмента
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, segmentIndex)));

    // Сохраняем количество вершин для отрисовки
    vertexCount = indices.size();

    // Освобождаем буферы
    vao.release();
    vbo.release();
    ibo.release();

    // Очищаем временные данные
    vertices.clear();
    indices.clear();
}

void EarthRenderer::render(const QMatrix4x4& projection, const QMatrix4x4& view, const QMatrix4x4& model)
{
    if (!program.isLinked()) return;

    program.bind();
    vao.bind();

    // Матрицы преобразования
    QMatrix4x4 mvp = projection * view * model;
    program.setUniformValue("mvp", mvp);
    program.setUniformValue("model", model);
    program.setUniformValue("normalMatrix", model.normalMatrix());

    // Настройка OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Отрисовка тайлов
    for (int ring = 0; ring < RINGS; ++ring) {
        for (int segment = 0; segment < SEGMENTS; ++segment) {
            // Установка текущего тайла
            program.setUniformValue("currentRing", ring);
            program.setUniformValue("currentSegment", segment);

            // Привязка текстуры тайла
            glActiveTexture(GL_TEXTURE0);
            if (earthTextureTiles->bindTileForSegment(ring, segment)) {
                program.setUniformValue("earthTexture", 0);

                // Отрисовка геометрии тайла
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT,
                               (void*)(ring * SEGMENTS * 6 * sizeof(GLuint) +
                                         segment * 6 * sizeof(GLuint)));
            }
        }
    }

    // Восстановление состояния OpenGL
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
