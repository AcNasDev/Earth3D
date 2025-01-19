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
    // Извлекаем только часть с поворотом из MVP матрицы
    mat4 modelView = mvp;
    // Сбрасываем масштабирование из матрицы вида
    modelView[0][0] = 1.0; modelView[0][1] = 0.0; modelView[0][2] = 0.0;
    modelView[1][0] = 0.0; modelView[1][1] = 1.0; modelView[1][2] = 0.0;
    modelView[2][0] = 0.0; modelView[2][1] = 0.0; modelView[2][2] = 1.0;

    // Применяем размер и позицию
    vec4 pos = modelView * vec4(billboardPos, 1.0);
    pos.xy += position.xy * billboardSize;
    gl_Position = pos;

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
    qDebug() << "Updating info texture for satellite ID:" << satellite.id;

    // Создаем расширенную информацию о спутнике
    QString info = QString("Satellite ID: %1\n%2")
                       .arg(satellite.id)
                       .arg(satellite.info);

    QImage textImage = createTextImage(info);
    qDebug() << "Created text image size:" << textImage.size();

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

    // Настройка параметров текстуры для лучшего качества
    texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    texture->generateMipMaps();

    qDebug() << "Successfully created texture with size:" << texture->width() << "x" << texture->height();
}

void SatelliteInfoRenderer::render(const QMatrix4x4& projection, const QMatrix4x4& view,
                                   const QMatrix4x4& model, const Satellite& satellite)
{
    if (!isInitialized || !texture) {
        qDebug() << "Not initialized or no texture";
        return;
    }

    program.bind();
    vao.bind();
    texture->bind();

    // Отключаем тест глубины для отображения поверх других объектов
    glDisable(GL_DEPTH_TEST);

    // Настройка прозрачности
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Вычисляем позицию с увеличенным смещением
    QVector3D offsetPos = satellite.position + QVector3D(0.0f, OFFSET, 0.0f);

    // Устанавливаем униформы
    QMatrix4x4 mvp = projection * view * model;
    program.setUniformValue("mvp", mvp);
    program.setUniformValue("billboardPos", offsetPos);
    program.setUniformValue("billboardSize", QVector2D(BILLBOARD_SIZE_X, BILLBOARD_SIZE_Y));
    program.setUniformValue("textTexture", 0);

    // Отрисовка
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // Восстанавливаем состояние OpenGL
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    texture->release();
    vao.release();
    program.release();
}

QImage SatelliteInfoRenderer::createTextImage(const QString& text)
{
    QFont font("Arial", 24, QFont::Bold);  // Увеличен размер шрифта
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

    // Увеличиваем отступы и размер изображения
    maxWidth += 40;  // Увеличенные отступы по бокам
    totalHeight += 40 + (lines.count() - 1) * 10;  // Увеличенные отступы сверху и снизу

    // Создаем изображение большего размера
    QImage image(maxWidth, totalHeight, QImage::Format_RGBA8888);
    image.fill(QColor(0, 0, 0, 180));  // Полупрозрачный черный фон для лучшей читаемости

    QPainter painter(&image);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.setRenderHint(QPainter::TextAntialiasing);

    // Рисуем текст
    int y = 20;  // Увеличенный отступ сверху
    for (const QString& line : lines) {
        painter.drawText(20, y + fm.ascent(), line);  // Увеличенный отступ слева
        y += fm.height() + 10;  // Увеличенный междустрочный интервал
    }

    painter.end();

    qDebug() << "Created texture size:" << image.size();
    return image;
}
