#include "satellite_info_renderer.h"
#include <QFontMetrics>
#include <QPainter>
#include <QDebug>  // Добавлено для отладки

SatelliteInfoRenderer::SatelliteInfoRenderer()
    : vbo(QOpenGLBuffer::VertexBuffer)
    , texture(nullptr)
    , isInitialized(false)
{
    // Перенесено в initialize()
}

SatelliteInfoRenderer::~SatelliteInfoRenderer()
{
    if (vbo.isCreated())
        vbo.destroy();
    if (texture) {
        texture->destroy();
        delete texture;
    }
}

bool SatelliteInfoRenderer::initialize()
{
    if (!initializeOpenGLFunctions()) {
        qDebug() << "Failed to initialize OpenGL functions";
        return false;
    }

    if (!initShaders()) {
        qDebug() << "Failed to initialize shaders";
        return false;
    }

    if (!initGeometry()) {
        qDebug() << "Failed to initialize geometry";
        return false;
    }

    isInitialized = true;
    return true;
}

bool SatelliteInfoRenderer::initShaders()
{
    // Vertex shader
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

    // Fragment shader
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

    // Проверка компиляции и линковки шейдеров
    if (!program.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource)) {
        qDebug() << "Vertex shader compilation failed:" << program.log();
        return false;
    }

    if (!program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource)) {
        qDebug() << "Fragment shader compilation failed:" << program.log();
        return false;
    }

    if (!program.link()) {
        qDebug() << "Shader program linking failed:" << program.log();
        return false;
    }

    return true;
}

bool SatelliteInfoRenderer::initGeometry()
{
    float vertices[] = {
        // Позиции    // Текстурные координаты
        -0.5f,  0.5f, 0.0f,   0.0f, 0.0f,
        0.5f,  0.5f, 0.0f,   1.0f, 0.0f,
        0.5f, -0.5f, 0.0f,   1.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,   0.0f, 1.0f
    };

    if (!vao.create()) {
        qDebug() << "Failed to create VAO";
        return false;
    }
    vao.bind();

    if (!vbo.create()) {
        qDebug() << "Failed to create VBO";
        return false;
    }
    vbo.bind();
    vbo.allocate(vertices, sizeof(vertices));

    // Проверяем, что программа активна
    if (!program.bind()) {
        qDebug() << "Failed to bind shader program";
        return false;
    }

    // Настройка атрибутов вершин
    program.enableAttributeArray(0);
    program.setAttributeBuffer(0, GL_FLOAT, 0, 3, 5 * sizeof(float));

    program.enableAttributeArray(1);
    program.setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 2, 5 * sizeof(float));

    vao.release();
    program.release();

    return true;
}

void SatelliteInfoRenderer::updateInfoTexture(const Satellite& satellite)
{
    QString info = QString("ID: %1\n%2").arg(satellite.id).arg(satellite.info);
    QImage textImage = createTextImage(info);

    if (texture) {
        texture->destroy();
        delete texture;
    }

    texture = new QOpenGLTexture(textImage);
    if (!texture->isCreated()) {
        qDebug() << "Failed to create texture";
        delete texture;
        texture = nullptr;
        return;
    }

    texture->setMinificationFilter(QOpenGLTexture::Linear);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);
}

void SatelliteInfoRenderer::render(const QMatrix4x4& projection, const QMatrix4x4& view,
                                   const QMatrix4x4& model, const Satellite& satellite)
{
    if (!isInitialized || !texture) {
        return;
    }

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

    // Включаем прозрачность
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Отрисовка
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // Отключаем прозрачность
    glDisable(GL_BLEND);

    texture->release();
    vao.release();
    program.release();
}

QImage SatelliteInfoRenderer::createTextImage(const QString& text)
{
    QFont font("Arial", 12, QFont::Bold);  // Сделаем шрифт жирным для лучшей видимости
    QFontMetrics fm(font);

    // Учитываем переносы строк в тексте
    QStringList lines = text.split('\n');
    int maxWidth = 0;
    int totalHeight = 0;

    for (const QString& line : lines) {
        QRect bounds = fm.boundingRect(line);
        maxWidth = qMax(maxWidth, bounds.width());
        totalHeight += bounds.height();
    }

    // Добавляем отступы
    maxWidth += 20;  // 10 пикселей с каждой стороны
    totalHeight += 20 + (lines.count() - 1) * 5;  // Отступы сверху и снизу + междустрочный интервал

    // Создаем изображение с прозрачным фоном
    QImage image(maxWidth, totalHeight, QImage::Format_RGBA8888);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.setRenderHint(QPainter::TextAntialiasing);

    // Рисуем текст
    int y = 10;
    for (const QString& line : lines) {
        painter.drawText(10, y + fm.ascent(), line);
        y += fm.height() + 5;  // Добавляем междустрочный интервал
    }

    painter.end();

    return image;
}
