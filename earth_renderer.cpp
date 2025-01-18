#include "earth_renderer.h"
#include <QImage>
#include <QtMath>
#include <QCoreApplication>
#include <QImageReader>

EarthRenderer::EarthRenderer(float radius)
    : Renderer()
    , earthTexture(nullptr)        // Добавить эту инициализацию
    , heightMapTexture(nullptr)    // Добавить эту инициализацию
    , normalMapTexture(nullptr)    // Добавить эту инициализацию
    , indexBuffer(QOpenGLBuffer::IndexBuffer)
    , radius(radius)
    , vertexCount(0)
    , displacementScale(0.05f)
{
}

EarthRenderer::~EarthRenderer()
{
    if (earthTexture)
        delete earthTexture;
    if (heightMapTexture)
        delete heightMapTexture;
    if (normalMapTexture)
        delete normalMapTexture;
    if (indexBuffer.isCreated())
        indexBuffer.destroy();
}

void EarthRenderer::initialize()
{
    if (!init()) {  // Вызов базового метода для инициализации OpenGL функций
        qDebug() << "Failed to initialize OpenGL functions for EarthRenderer";
        return;
    }

    initShaders();
    initTextures();
    initGeometry();
}

void EarthRenderer::initShaders()
{
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/earth_vertex.glsl"))
        qDebug() << "Failed to compile earth vertex shader";

    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/earth_fragment.glsl"))
        qDebug() << "Failed to compile earth fragment shader";

    if (!program.link())
        qDebug() << "Failed to link earth shader program";
}

void EarthRenderer::initTextures()
{
    QImageReader::setAllocationLimit(0);
    QString buildDir = QCoreApplication::applicationDirPath();

    // Загрузка текстуры Земли
    QImage earthImage(buildDir + "/textures/earth.jpg");
    if (!earthImage.isNull()) {
        earthTexture = new QOpenGLTexture(earthImage.mirrored());
        if (!earthTexture->isCreated()) {
            delete earthTexture;
            earthTexture = nullptr;
            qDebug() << "Failed to create earth texture";
        } else {
            earthTexture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
            earthTexture->setMagnificationFilter(QOpenGLTexture::Linear);
            earthTexture->setWrapMode(QOpenGLTexture::Repeat);
        }
    }

    // Загрузка карты высот
    QImage heightImage(buildDir + "/textures/earth_height.png");
    if (!heightImage.isNull()) {
        heightMapTexture = new QOpenGLTexture(heightImage.mirrored());
        if (!heightMapTexture->isCreated()) {
            delete heightMapTexture;
            heightMapTexture = nullptr;
            qDebug() << "Failed to create height map texture";
        } else {
            heightMapTexture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
            heightMapTexture->setMagnificationFilter(QOpenGLTexture::Linear);
            heightMapTexture->setWrapMode(QOpenGLTexture::Repeat);
        }
    }

    // Загрузка карты нормалей
    QImage normalImage(buildDir + "/textures/earth_normal.png");
    if (!normalImage.isNull()) {
        normalMapTexture = new QOpenGLTexture(normalImage.mirrored());
        if (!normalMapTexture->isCreated()) {
            delete normalMapTexture;
            normalMapTexture = nullptr;
            qDebug() << "Failed to create normal map texture";
        } else {
            normalMapTexture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
            normalMapTexture->setMagnificationFilter(QOpenGLTexture::Linear);
            normalMapTexture->setWrapMode(QOpenGLTexture::Repeat);
        }
    }
}

void EarthRenderer::initGeometry()
{
    vao.create();
    vao.bind();

    // Создаем буферы
    vbo.create();
    indexBuffer.create();

    // Создаем геометрию сферы
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

            // Позиция
            vertices << x << y << z;

            // Текстурные координаты
            vertices << (1.0f - static_cast<float>(segment) / segments)
                     << (1.0f - static_cast<float>(ring) / rings);

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), nullptr);

    glEnableVertexAttribArray(1); // texcoord
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                          reinterpret_cast<void*>(3 * sizeof(GLfloat)));

    glEnableVertexAttribArray(2); // normal
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                          reinterpret_cast<void*>(5 * sizeof(GLfloat)));
}

void EarthRenderer::render(const QMatrix4x4& projection, const QMatrix4x4& view, const QMatrix4x4& model)
{
    program.bind();
    vao.bind();

    // Создаем матрицу для Земли
    QMatrix4x4 earthMatrix = model;
    earthMatrix.scale(radius);

    // Устанавливаем униформы
    program.setUniformValue("mvp", projection * view * earthMatrix);
    program.setUniformValue("model", earthMatrix);
    program.setUniformValue("normalMatrix", earthMatrix.normalMatrix());
    program.setUniformValue("viewPos", view.inverted().column(3).toVector3D());
    program.setUniformValue("displacementScale", displacementScale);

    // Привязываем текстуры
    if (earthTexture) {
        earthTexture->bind(0);
        program.setUniformValue("earthTexture", 0);
    }

    if (heightMapTexture) {
        heightMapTexture->bind(1);
        program.setUniformValue("heightMap", 1);
    }

    if (normalMapTexture) {
        normalMapTexture->bind(2);
        program.setUniformValue("normalMap", 2);
    }

    // Рендерим сферу
    glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, nullptr);

    // Очищаем состояние
    vao.release();
    program.release();
}
