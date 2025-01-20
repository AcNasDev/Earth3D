#include "atmosphere_renderer.h"
#include <QtMath>
#include <qapplication.h>

AtmosphereRenderer::AtmosphereRenderer(float earthRadius)
    : Renderer()
    , radius(earthRadius)
{
}

AtmosphereRenderer::~AtmosphereRenderer() = default;

void AtmosphereRenderer::initialize() {
    if (!init()) {
        qDebug() << "Failed to initialize OpenGL functions for EarthRenderer";
        return;
    }
    initShaders();
    initTextures();  // Добавляем инициализацию текстур
    initGeometry();
}

void AtmosphereRenderer::initTextures() {
    QString buildDir = QCoreApplication::applicationDirPath();

    skyTexture = std::make_unique<TileTextureManager>(
        buildDir + "/textures/earth_clouds.jpg", RINGS, SEGMENTS);
    skyTexture->initialize();
}

void AtmosphereRenderer::initShaders() {
    program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/atmosphere_vertex.glsl");
    program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/atmosphere_fragment.glsl");
    program.link();
}

void AtmosphereRenderer::render(const QMatrix4x4& projection, const QMatrix4x4& view, const QMatrix4x4& model) {
    if (!program.bind())
        return;

    time += 0.016f; // Примерно 60 FPS

    vao.bind();

    QVector3D cameraPos = view.inverted().column(3).toVector3D();

    program.setUniformValue("projectionMatrix", projection);
    program.setUniformValue("viewMatrix", view);
    program.setUniformValue("modelMatrix", model);
    program.setUniformValue("viewPos", cameraPos);
    program.setUniformValue("lightPos", cameraPos);
    program.setUniformValue("time", time);

    // Привязываем текстуру неба
    glActiveTexture(GL_TEXTURE0);
    skyTexture->bindTileTexture(0, 0);
    program.setUniformValue("skyTexture", 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    vao.release();
    program.release();
}

void AtmosphereRenderer::initGeometry() {
    createSphere();

    vao.create();
    vao.bind();

    // Создаем и заполняем VBO
    vbo.create();
    vbo.bind();
    vbo.allocate(vertices.constData(), vertices.size() * sizeof(Vertex));

    // Создаем и заполняем IBO
    ibo.create();
    ibo.bind();
    ibo.allocate(indices.constData(), indices.size() * sizeof(GLuint));

    // Указываем атрибуты вершин
    program.enableAttributeArray("position");
    program.setAttributeBuffer("position", GL_FLOAT, offsetof(Vertex, position), 3, sizeof(Vertex));

    program.enableAttributeArray("texCoord");
    program.setAttributeBuffer("texCoord", GL_FLOAT, offsetof(Vertex, texCoord), 2, sizeof(Vertex));

    program.enableAttributeArray("normal");
    program.setAttributeBuffer("normal", GL_FLOAT, offsetof(Vertex, normal), 3, sizeof(Vertex));

    vao.release();
}

void AtmosphereRenderer::createSphere() {
    vertices.clear();
    indices.clear();

    for (int ring = 0; ring <= RINGS; ++ring) {
        float phi = M_PI * float(ring) / RINGS;
        for (int segment = 0; segment <= SEGMENTS; ++segment) {
            float theta = 2.0f * M_PI * float(segment) / SEGMENTS;

            QVector3D position = sphericalToCartesian(radius, phi, theta);
            QVector3D normal = position.normalized();
            float u = float(segment) / SEGMENTS;
            float v = float(ring) / RINGS;

            vertices.append({position, QVector2D(u, v), normal});

            if (ring < RINGS && segment < SEGMENTS) {
                int first = ring * (SEGMENTS + 1) + segment;
                int second = first + SEGMENTS + 1;

                indices.append(first);
                indices.append(second);
                indices.append(first + 1);

                indices.append(second);
                indices.append(second + 1);
                indices.append(first + 1);
            }
        }
    }
}

QVector3D AtmosphereRenderer::sphericalToCartesian(float radius, float phi, float theta) const {
    float x = radius * sin(phi) * cos(theta);
    float y = radius * cos(phi);
    float z = radius * sin(phi) * sin(theta);
    return QVector3D(x, y, z);
}
