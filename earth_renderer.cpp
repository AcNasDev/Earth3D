// earth_renderer.cpp
#include "earth_renderer.h"
#include <QDebug>
#include <QtMath>
#include <QCoreApplication>

EarthRenderer::EarthRenderer(float earthRadius)
    : radius(earthRadius)
{
}

EarthRenderer::~EarthRenderer() {
    if (vbo.isCreated())
        vbo.destroy();
    if (ibo.isCreated())
        ibo.destroy();
    if (vao.isCreated())
        vao.destroy();
}

void EarthRenderer::initialize() {
    if (!init()) {
        qDebug() << "Failed to initialize OpenGL functions for EarthRenderer";
        return;
    }

    initShaders();
    initTextures();
    initGeometry();
}

void EarthRenderer::initShaders() {
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

void EarthRenderer::initTextures() {
    QString buildDir = QCoreApplication::applicationDirPath();

    earthTextureTiles = std::make_unique<TileTextureManager>(
        buildDir + "/textures/earth.jpg", RINGS, SEGMENTS);
    heightMapTiles = std::make_unique<TileTextureManager>(
        buildDir + "/textures/earth_height.png", RINGS, SEGMENTS);
    normalMapTiles = std::make_unique<TileTextureManager>(
        buildDir + "/textures/earth_normal.png", RINGS, SEGMENTS);

    earthTextureTiles->initialize();
    heightMapTiles->initialize();
    normalMapTiles->initialize();
}

void EarthRenderer::initGeometry() {
    vao.create();
    vao.bind();

    vbo.create();
    ibo.create();

    createSphere();

    vao.release();
}

void EarthRenderer::render(const QMatrix4x4& projection, const QMatrix4x4& view, const QMatrix4x4& model) {
    if (!program.bind())
        return;

    vao.bind();

    program.setUniformValue("projectionMatrix", projection);
    program.setUniformValue("viewMatrix", view);
    program.setUniformValue("modelMatrix", model);

    // Привязываем все текстуры один раз
    glActiveTexture(GL_TEXTURE0);
    earthTextureTiles->bindTileTexture(0, 0);  // Привязываем атлас текстур
    program.setUniformValue("earthTexture", 0);

    glActiveTexture(GL_TEXTURE1);
    heightMapTiles->bindTileTexture(0, 0);
    program.setUniformValue("heightMap", 1);

    glActiveTexture(GL_TEXTURE2);
    normalMapTiles->bindTileTexture(0, 0);
    program.setUniformValue("normalMap", 2);

    // Рендерим всю геометрию за один draw call
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

    vao.release();
    program.release();
}

void EarthRenderer::createSphere() {
    vertices.clear();
    indices.clear();

    for (int ring = 0; ring < RINGS; ++ring) {
        float phi1 = M_PI * float(ring) / RINGS;
        float phi2 = M_PI * float(ring + 1) / RINGS;

        for (int segment = 0; segment < SEGMENTS; ++segment) {
            float theta1 = 2.0f * M_PI * float(segment) / SEGMENTS;
            float theta2 = 2.0f * M_PI * float(segment + 1) / SEGMENTS;

            QVector3D v1 = sphericalToCartesian(radius, phi1, theta1);
            QVector3D v2 = sphericalToCartesian(radius, phi1, theta2);
            QVector3D v3 = sphericalToCartesian(radius, phi2, theta2);
            QVector3D v4 = sphericalToCartesian(radius, phi2, theta1);

            // Получаем UV-координаты из атласа текстур
            QRectF uvCoords = earthTextureTiles->getTileUVCoords(ring, segment);
            QVector2D uv1(uvCoords.left(), uvCoords.top());
            QVector2D uv2(uvCoords.right(), uvCoords.top());
            QVector2D uv3(uvCoords.right(), uvCoords.bottom());
            QVector2D uv4(uvCoords.left(), uvCoords.bottom());

            // Нормали
            QVector3D n1 = v1.normalized();
            QVector3D n2 = v2.normalized();
            QVector3D n3 = v3.normalized();
            QVector3D n4 = v4.normalized();

            int baseIndex = vertices.size();
            vertices.append(Vertex{v1, uv1, n1, QVector2D(ring, segment)});
            vertices.append(Vertex{v2, uv2, n2, QVector2D(ring, segment)});
            vertices.append(Vertex{v3, uv3, n3, QVector2D(ring, segment)});
            vertices.append(Vertex{v4, uv4, n4, QVector2D(ring, segment)});

            indices.append(baseIndex);
            indices.append(baseIndex + 1);
            indices.append(baseIndex + 2);
            indices.append(baseIndex);
            indices.append(baseIndex + 2);
            indices.append(baseIndex + 3);
        }
    }

    vbo.bind();
    vbo.allocate(vertices.constData(), vertices.size() * sizeof(Vertex));

    ibo.bind();
    ibo.allocate(indices.constData(), indices.size() * sizeof(GLuint));

    program.enableAttributeArray("position");
    program.setAttributeBuffer("position", GL_FLOAT, offsetof(Vertex, position), 3, sizeof(Vertex));

    program.enableAttributeArray("texCoord");
    program.setAttributeBuffer("texCoord", GL_FLOAT, offsetof(Vertex, texCoord), 2, sizeof(Vertex));

    program.enableAttributeArray("normal");
    program.setAttributeBuffer("normal", GL_FLOAT, offsetof(Vertex, normal), 3, sizeof(Vertex));

    program.enableAttributeArray("tileCoord");
    program.setAttributeBuffer("tileCoord", GL_FLOAT, offsetof(Vertex, tileCoord), 2, sizeof(Vertex));
}

void EarthRenderer::updateVisibleTiles(const QMatrix4x4& viewProjection) {
    earthTextureTiles->updateVisibleTiles(viewProjection);
    heightMapTiles->updateVisibleTiles(viewProjection);
    normalMapTiles->updateVisibleTiles(viewProjection);
}

QVector3D EarthRenderer::sphericalToCartesian(float radius, float phi, float theta) const {
    float x = radius * sin(phi) * cos(theta);
    float y = radius * cos(phi);
    float z = radius * sin(phi) * sin(theta);
    return QVector3D(x, y, z);
}
