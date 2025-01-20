#include "atmosphere_renderer.h"
#include <QtMath>
#include <qapplication.h>

AtmosphereRenderer::AtmosphereRenderer(float earthRadius)
    : Renderer()
    , radius(earthRadius * 1.15f)
{
}

AtmosphereRenderer::~AtmosphereRenderer() = default;

void AtmosphereRenderer::initialize() {
    if (!init()) {
        qDebug() << "Failed to initialize OpenGL functions for EarthRenderer";
        return;
    }
    initShaders();
    initTextures();  // Добавляем вызов initTextures
    initGeometry();
}

void AtmosphereRenderer::initTextures() {
    QString buildDir = QCoreApplication::applicationDirPath();
    skyTexture = std::make_unique<TileTextureManager>(
        buildDir + "/textures/earth_clouds.jpg", RINGS, SEGMENTS);
    skyTexture->initialize();

    // Устанавливаем параметры текстурирования
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void AtmosphereRenderer::initShaders() {
    program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/atmosphere_vertex.glsl");
    program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/atmosphere_fragment.glsl");
    program.link();
}

void AtmosphereRenderer::render(const QMatrix4x4& projection, const QMatrix4x4& view, const QMatrix4x4& model) {
    if (!program.bind())
        return;

    time += 0.016f;

    vao.bind();

    QVector3D cameraPos = view.inverted().column(3).toVector3D();

    program.setUniformValue("projectionMatrix", projection);
    program.setUniformValue("viewMatrix", view);
    program.setUniformValue("modelMatrix", model);
    program.setUniformValue("viewPos", cameraPos);
    program.setUniformValue("lightPos", cameraPos);
    program.setUniformValue("time", time);

    // Привязываем текстуру облаков
    glActiveTexture(GL_TEXTURE0);
    skyTexture->bindTileTexture(0, 0);
    program.setUniformValue("skyTexture", 0);

    // Настраиваем состояния OpenGL для правильного отображения неба
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE); // Отключаем запись в буфер глубины для прозрачности

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT); // Меняем на FRONT для отображения внутренней стороны сферы

    // Рисуем атмосферу
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

    // Восстанавливаем состояния
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glCullFace(GL_BACK);

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

            // Корректируем текстурные координаты
            float u = float(segment) / SEGMENTS;
            float v = 1.0f - float(ring) / RINGS; // Инвертируем v координату

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
