// trajectory_renderer.cpp
#include "trajectory_renderer.h"

TrajectoryRenderer::TrajectoryRenderer()
{
    vbo.create();
}

TrajectoryRenderer::~TrajectoryRenderer()
{
    vbo.destroy();
}

void TrajectoryRenderer::initialize()
{
    initializeOpenGLFunctions();
    initShaders();

    vao.create();
    vao.bind();
    vbo.bind();

    // Настраиваем атрибут позиции
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), nullptr);

    vao.release();
    vbo.release();
}

void TrajectoryRenderer::initShaders()
{
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/line_vertex.glsl"))
        qDebug() << "Failed to compile vertex shader";

    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/line_fragment.glsl"))
        qDebug() << "Failed to compile fragment shader";

    if (!program.link())
        qDebug() << "Failed to link shader program";
}

void TrajectoryRenderer::setTrajectories(const QVector<QVector3D>& current,
                                         const QVector<QVector3D>& predicted)
{
    currentTrajectory = current;
    predictedTrajectory = predicted;
}

void TrajectoryRenderer::render(const QMatrix4x4& projection, const QMatrix4x4& view, const QMatrix4x4& model)
{
    if (currentTrajectory.isEmpty() && predictedTrajectory.isEmpty())
        return;

    QMatrix4x4 mvp = projection * view * model;

    program.bind();
    vao.bind();

    // Настройка состояния OpenGL
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);

    GLint previousDepthFunc;
    glGetIntegerv(GL_DEPTH_FUNC, &previousDepthFunc);
    glDepthFunc(GL_LEQUAL);

    // Отрисовка текущей траектории (пунктирная белая линия)
    if (!currentTrajectory.isEmpty()) {
        QVector<QVector3D> dashedPoints;
        createDashedLine(currentTrajectory, dashedPoints);
        renderTrajectory(dashedPoints,
                         QVector4D(1.0f, 1.0f, 1.0f, 1.0f),  // Белый цвет
                         true,
                         mvp);
    }

    // Отрисовка предсказанной траектории (сплошная голубая линия)
    if (!predictedTrajectory.isEmpty()) {
        renderTrajectory(predictedTrajectory,
                         QVector4D(0.0f, 1.0f, 1.0f, 0.5f),  // Полупрозрачный голубой
                         false,
                         mvp);
    }

    // Восстановление состояния OpenGL
    glDepthFunc(previousDepthFunc);
    vao.release();
    program.release();
}

void TrajectoryRenderer::createDashedLine(const QVector<QVector3D>& sourcePoints,
                                          QVector<QVector3D>& dashedPoints)
{
    dashedPoints.clear();

    for (int i = 0; i < sourcePoints.size() - 1; i++) {
        QVector3D start = sourcePoints[i];
        QVector3D end = sourcePoints[i + 1];
        QVector3D segment = end - start;
        float length = segment.length();
        QVector3D direction = segment.normalized();

        float accumulatedLength = 0.0f;
        bool isDash = true;

        while (accumulatedLength < length) {
            float currentLength = isDash ? dashLength : gapLength;

            if (accumulatedLength + currentLength > length) {
                currentLength = length - accumulatedLength;
            }

            if (isDash) {
                dashedPoints.append(start + direction * accumulatedLength);
                dashedPoints.append(start + direction * (accumulatedLength + currentLength));
            }

            accumulatedLength += currentLength;
            isDash = !isDash;
        }
    }
}

void TrajectoryRenderer::renderTrajectory(const QVector<QVector3D>& points,
                                          const QVector4D& color,
                                          bool isDashed,
                                          const QMatrix4x4& mvp)
{
    vbo.bind();
    vbo.allocate(points.constData(), points.size() * sizeof(QVector3D));

    program.setUniformValue("mvp", mvp);
    program.setUniformValue("color", color);

    glLineWidth(isDashed ? 2.0f : 1.5f);
    glDrawArrays(isDashed ? GL_LINES : GL_LINE_STRIP, 0, points.size());

    vbo.release();
}
