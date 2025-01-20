#include "satellite_renderer.h"
#include <QtMath>

SatelliteRenderer::SatelliteRenderer()
    : Renderer()
    , indexBuffer(QOpenGLBuffer::IndexBuffer)
    , vertexCount(0)
{
    time = 0.0f;
}

SatelliteRenderer::~SatelliteRenderer()
{
    if (indexBuffer.isCreated())
        indexBuffer.destroy();
}

void SatelliteRenderer::initialize()
{
    initShaders();
    initGeometry();
}

void SatelliteRenderer::initShaders()
{
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/sat_vertex.glsl"))
        qDebug() << "Failed to compile satellite vertex shader";

    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/sat_fragment.glsl"))
        qDebug() << "Failed to compile satellite fragment shader";

    if (!program.link())
        qDebug() << "Failed to link satellite shader program";
}

void SatelliteRenderer::initGeometry()
{
    vao.create();
    vao.bind();

    vbo.create();
    indexBuffer.create();

    createSphere(RINGS, SEGMENTS);

    vao.release();
}

void SatelliteRenderer::createSphere(int rings, int segments)
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

            // Позиция
            vertices << x << y << z;

            // Нормали (совпадают с позицией для сферы)
            vertices << x << y << z;
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), nullptr);

    glEnableVertexAttribArray(1); // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
                          reinterpret_cast<void*>(3 * sizeof(GLfloat)));
}

void SatelliteRenderer::updateSatellites(const QMap<int, Satellite>& newSatellites)
{
    satellites = newSatellites;
}

void SatelliteRenderer::render(const QMatrix4x4& projection, const QMatrix4x4& view, const QMatrix4x4& model)
{
    program.bind();
    vao.bind();

    // Включаем прозрачность и сглаживание
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    for (const auto& satellite : satellites) {
        qDebug() << satellite.id << satellite.isSelected;
        QMatrix4x4 satMatrix = model;
        satMatrix.translate(satellite.position);

        // Вычисляем расстояние до камеры для масштабирования
        QVector3D satelliteWorldPos = model * satellite.position;
        QVector3D cameraPos = view.inverted().column(3).toVector3D();
        QVector3D toCameraVector = cameraPos - satelliteWorldPos;
        float distanceToCamera = toCameraVector.length();

        // Масштабируем размер спутника пропорционально расстоянию
        float scale = distanceToCamera * 0.005f;
        satMatrix.scale(scale);

        // Устанавливаем униформы
        program.setUniformValue("mvp", projection * view * satMatrix);
        program.setUniformValue("model", satMatrix);
        program.setUniformValue("normalMatrix", satMatrix.normalMatrix());
        program.setUniformValue("viewPos", cameraPos);
        program.setUniformValue("isSelected", satellite.isSelected);
        time += 0.016f; // Примерно 60 FPS
        program.setUniformValue("time", time);

        // Рендерим спутник
        glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, nullptr);
    }

    // Восстанавливаем состояние OpenGL
    glDisable(GL_BLEND);

    vao.release();
    program.release();
}
