// progress_bar.cpp
#include "progress_bar.h"
#include <QVector>

ProgressBar::ProgressBar()
    : Renderer()
    , progress(0.0f)
    , indexBuffer(QOpenGLBuffer::IndexBuffer)
{
}

ProgressBar::~ProgressBar()
{
    if (indexBuffer.isCreated())
        indexBuffer.destroy();
}

void ProgressBar::initialize()
{
    if (!init()) {
        qDebug() << "Failed to initialize OpenGL functions for ProgressBar";
        return;
    }
    initShaders();
    initGeometry();
}

void ProgressBar::initShaders()
{
    // Create and compile shaders for progress bar
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;
        uniform mat4 projection;
        uniform mat4 view;
        uniform mat4 model;
        out vec2 TexCoord;
        void main()
        {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
            TexCoord = aTexCoord;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform float progress;
        void main()
        {
            if (TexCoord.x <= progress)
                FragColor = vec4(0.2, 0.6, 1.0, 0.8); // Progress color
            else
                FragColor = vec4(0.3, 0.3, 0.3, 0.5); // Background color
        }
    )";

    program.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    program.link();
}

void ProgressBar::initGeometry()
{
    vao.create();
    vao.bind();

    // Create a simple quad for the progress bar
    QVector<GLfloat> vertices = {
        // Positions        // TexCoords
        -0.8f,  0.8f, 0.0f,  0.0f, 1.0f,
        -0.8f,  0.7f, 0.0f,  0.0f, 0.0f,
        0.8f,  0.8f, 0.0f,  1.0f, 1.0f,
        0.8f,  0.7f, 0.0f,  1.0f, 0.0f
    };

    QVector<GLuint> indices = {
        0, 1, 2,
        1, 3, 2
    };

    vbo.create();
    vbo.bind();
    vbo.allocate(vertices.constData(), vertices.size() * sizeof(GLfloat));

    indexBuffer.create();
    indexBuffer.bind();
    indexBuffer.allocate(indices.constData(), indices.size() * sizeof(GLuint));

    program.enableAttributeArray(0);
    program.setAttributeBuffer(0, GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));

    program.enableAttributeArray(1);
    program.setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));

    vao.release();
}

void ProgressBar::setProgress(float value)
{
    progress = qBound(0.0f, value, 1.0f);
}

void ProgressBar::render(const QMatrix4x4& projection, const QMatrix4x4& view, const QMatrix4x4& model)
{
    program.bind();
    vao.bind();

    program.setUniformValue("projection", projection);
    program.setUniformValue("view", view);
    program.setUniformValue("model", model);
    program.setUniformValue("progress", progress);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    vao.release();
    program.release();
}
