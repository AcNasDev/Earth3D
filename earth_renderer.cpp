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

    for (int ring = 0; ring <= RINGS; ++ring) {
        float phi = ring * M_PI / RINGS;
        float v = static_cast<float>(ring) / RINGS;

        for (int segment = 0; segment <= SEGMENTS; ++segment) {
            float theta = segment * 2.0f * M_PI / SEGMENTS;
            float u = 1.0f - static_cast<float>(segment) / SEGMENTS;

            // Позиция
            QVector3D pos(
                sin(phi) * cos(theta) * radius,
                cos(phi) * radius,
                sin(phi) * sin(theta) * radius
                );

            // Нормаль
            QVector3D normal(
                sin(phi) * cos(theta),
                cos(phi),
                sin(phi) * sin(theta)
                );

            Vertex vertex;
            vertex.position = pos;
            vertex.texCoord = QVector2D(u, v);
            vertex.normal = normal;
            vertex.segmentIndex = QVector2D(ring, segment);

            vertices.append(vertex);
        }
    }

    // Генерация индексов
    for (int ring = 0; ring < RINGS; ++ring) {
        for (int segment = 0; segment < SEGMENTS; ++segment) {
            GLuint current = ring * (SEGMENTS + 1) + segment;
            GLuint next = current + SEGMENTS + 1;

            indices << current << current + 1 << next;
            indices << next << current + 1 << next + 1;
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

void EarthRenderer::render(const QMatrix4x4& projection, const QMatrix4x4& view, const QMatrix4x4& model)
{
    if (!program.isLinked()) return;

    program.bind();
    vao.bind();

    // Базовые униформы
    QMatrix4x4 mvp = projection * view * model;
    program.setUniformValue("mvp", mvp);
    program.setUniformValue("model", model);
    program.setUniformValue("normalMatrix", model.normalMatrix());

    // Настройка OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Рендеринг каждого тайла
    for (int ring = 0; ring < RINGS; ++ring) {
        for (int segment = 0; segment < SEGMENTS; ++segment) {
            // Установка текущего тайла
            program.setUniformValue("currentRing", ring);
            program.setUniformValue("currentSegment", segment);

            // Привязка текстуры
            glActiveTexture(GL_TEXTURE0);
            earthTextureTiles->bindTileForSegment(ring, segment);
            glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, nullptr);
        }
    }

    glDisable(GL_BLEND);
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
