#include "satellite_info_renderer.h"
#include <QFontMetrics>
#include <QPainter>

SatelliteInfoRenderer::SatelliteInfoRenderer()
    : vbo(QOpenGLBuffer::VertexBuffer)
    , texture(nullptr)
{
}

SatelliteInfoRenderer::~SatelliteInfoRenderer()
{
    if (vbo.isCreated())
        vbo.destroy();
    delete texture;
}

void SatelliteInfoRenderer::initialize()
{
    initShaders();
    initGeometry();
}

void SatelliteInfoRenderer::initShaders()
{
    // Vertex shader для billboard эффекта
    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec2 texCoord;

        uniform mat4 mvp;
        uniform vec3 billboardPos;
        uniform vec2 billboardSize;

        out vec2 TexCoord;

        void main()
        {
            vec3 pos = billboardPos + vec3(position.x * billboardSize.x,
                                         position.y * billboardSize.y, 0.0);
            gl_Position = mvp * vec4(pos, 1.0);
            TexCoord = texCoord;
        }
    )";

    // Fragment shader для отрисовки текстуры
    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;

        uniform sampler2D textTexture;

        void main()
        {
            FragColor = texture(textTexture, TexCoord);
        }
    )";

    program.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    program.link();
}

void SatelliteInfoRenderer::initGeometry()
{
    // Создаем простой прямоугольник для отрисовки текстуры
    float vertices[] = {
        // Позиции    // Текстурные координаты
        -0.5f,  0.5f, 0.0f,   0.0f, 0.0f,
        0.5f,  0.5f, 0.0f,   1.0f, 0.0f,
        0.5f, -0.5f, 0.0f,   1.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,   0.0f, 1.0f
    };

    vao.create();
    vao.bind();

    vbo.create();
    vbo.bind();
    vbo.allocate(vertices, sizeof(vertices));

    // Настройка атрибутов вершин
    program.enableAttributeArray(0);
    program.setAttributeBuffer(0, GL_FLOAT, 0, 3, 5 * sizeof(float));

    program.enableAttributeArray(1);
    program.setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 2, 5 * sizeof(float));

    vao.release();
}

QImage SatelliteInfoRenderer::createTextImage(const QString& text)
{
    QFont font("Arial", 12);
    QFontMetrics fm(font);
    QSize textSize = fm.size(Qt::TextSingleLine, text);

    // Создаем изображение с прозрачным фоном
    QImage image(textSize.width() + 10, textSize.height() + 10, QImage::Format_RGBA8888);
    image.fill(Qt::transparent);

    // Используем QPainter только для создания текстуры
    QPainter painter(&image);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(5, fm.ascent() + 5, text);
    painter.end();

    return image;
}

void SatelliteInfoRenderer::updateInfoTexture(const Satellite& satellite)
{
    QString info = QString("ID: %1\n%2").arg(satellite.id).arg(satellite.info);
    QImage textImage = createTextImage(info);

    delete texture;
    texture = new QOpenGLTexture(textImage);
    texture->setMinificationFilter(QOpenGLTexture::Linear);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
}

void SatelliteInfoRenderer::render(const QMatrix4x4& projection, const QMatrix4x4& view,
                                   const QMatrix4x4& model, const Satellite& satellite)
{
    if (!texture)
        updateInfoTexture(satellite);

    program.bind();
    vao.bind();
    texture->bind();

    // Вычисляем позицию billboard'а
    QVector3D offsetPos = satellite.position + QVector3D(0.0f, OFFSET, 0.0f);

    // Устанавливаем униформы
    program.setUniformValue("mvp", projection * view * model);
    program.setUniformValue("billboardPos", offsetPos);
    program.setUniformValue("billboardSize", QVector2D(BILLBOARD_SIZE, BILLBOARD_SIZE));
    program.setUniformValue("textTexture", 0);

    // Отрисовка
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisable(GL_BLEND);

    texture->release();
    vao.release();
    program.release();
}
