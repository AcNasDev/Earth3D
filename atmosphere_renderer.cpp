#include "atmosphere_renderer.h"
#include <QtMath>
#include <qapplication.h>

AtmosphereRenderer::AtmosphereRenderer(float earthRadius)
    : Renderer()
    , radius(earthRadius * 1.05f)
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

    update(0.016f);

    vao.bind();

    QVector3D cameraPos = view.inverted().column(3).toVector3D();

    program.setUniformValue("projectionMatrix", projection);
    program.setUniformValue("viewMatrix", view);
    program.setUniformValue("modelMatrix", model);
    program.setUniformValue("viewPos", cameraPos);
    program.setUniformValue("lightPos", cameraPos);
    program.setUniformValue("cloudRotationMatrix", cloudRotationMatrix);

    // Привязываем текстуру облаков
    glActiveTexture(GL_TEXTURE0);
    skyTexture->bindTileTexture(0, 0);
    program.setUniformValue("skyTexture", 0);

    // Сохраняем текущие состояния OpenGL
    GLboolean depthTest, blend, cullFace;
    GLint depthFunc;
    glGetBooleanv(GL_DEPTH_TEST, &depthTest);
    glGetBooleanv(GL_BLEND, &blend);
    glGetBooleanv(GL_CULL_FACE, &cullFace);
    glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);

    // Настраиваем состояния для рендеринга атмосферы
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);  // Важно: используем LEQUAL вместо LESS
    glDepthMask(GL_FALSE);   // Отключаем запись в буфер глубины

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);     // Отсекаем задние грани
    glFrontFace(GL_CCW);     // Порядок вершин против часовой стрелки

    // Рисуем атмосферу
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

    // Восстанавливаем состояния OpenGL
    glDepthFunc(depthFunc);
    glDepthMask(GL_TRUE);
    if (!depthTest) glDisable(GL_DEPTH_TEST);
    if (!blend) glDisable(GL_BLEND);
    if (!cullFace) glDisable(GL_CULL_FACE);

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
            QRectF uvCoords = skyTexture->getTileUVCoords(ring, segment);
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
            vertices.append({v1, uv1, n1});
            vertices.append({v2, uv2, n2});
            vertices.append({v3, uv3, n3});
            vertices.append({v4, uv4, n4});

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
}

QVector3D AtmosphereRenderer::sphericalToCartesian(float radius, float phi, float theta) const {
    float x = radius * sin(phi) * cos(theta);
    float y = radius * cos(phi);
    float z = radius * sin(phi) * sin(theta);
    return QVector3D(x, y, z);
}

void AtmosphereRenderer::update(float deltaTime) {
    // Обновляем угол вращения (настройте скорость по необходимости)
    float rotationSpeed = 0.02f; // радиан в секунду
    rotationAngle += rotationSpeed * deltaTime;

    // Создаем матрицу вращения вокруг оси Y
    cloudRotationMatrix.setToIdentity();
    cloudRotationMatrix.rotate(QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0),
                                                             qRadiansToDegrees(rotationAngle)));
}
