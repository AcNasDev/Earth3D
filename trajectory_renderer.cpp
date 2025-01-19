// trajectory_renderer.cpp
#include "trajectory_renderer.h"

TrajectoryRenderer::TrajectoryRenderer()
    : currentVBO(QOpenGLBuffer::VertexBuffer),
    predictedVBO(QOpenGLBuffer::VertexBuffer),
    currentVertexCount(0),
    predictedVertexCount(0),
    needsUpdate(false)
{
}

TrajectoryRenderer::~TrajectoryRenderer()
{
    if (currentVBO.isCreated())
        currentVBO.destroy();
    if (predictedVBO.isCreated())
        predictedVBO.destroy();
}

void TrajectoryRenderer::initialize()
{
    initializeOpenGLFunctions();
    initShaders();

    // Создаем VAO и VBO
    vao.create();
    vao.bind();

    currentVBO.create();
    predictedVBO.create();

    vao.release();
}

void TrajectoryRenderer::initShaders()
{
    // Создаем и компилируем шейдеры
    if (!program.addShaderFromSourceCode(QOpenGLShader::Vertex,
                                         "# version 330 core\n"
                                         "layout(location = 0) in vec3 aPos;\n"
                                         "uniform mat4 mvp;\n"
                                         "void main() {\n"
                                         "    gl_Position = mvp * vec4(aPos, 1.0);\n"
                                         "}\n"))
        qDebug() << "Не удалось скомпилировать вертексный шейдер";

    if (!program.addShaderFromSourceCode(QOpenGLShader::Fragment,
                                         "#version 330 core\n"
                                         "out vec4 FragColor;\n"
                                         "uniform vec4 color;\n"
                                         "void main() {\n"
                                         "    FragColor = color;\n"
                                         "}\n"))
        qDebug() << "Не удалось скомпилировать фрагментный шейдер";

    if (!program.link())
        qDebug() << "Не удалось слинковать шейдерную программу";
}

void TrajectoryRenderer::setTrajectories(const QVector<QVector3D>& current,
                                         const QVector<QVector3D>& predicted)
{
    if (current != currentTrajectory || predicted != predictedTrajectory) {
        currentTrajectory = current;
        predictedTrajectory = predicted;
        needsUpdate = true;
    }
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
    glLineWidth(2.0f);  // Делаем линии толще для лучшей видимости

    GLint previousDepthFunc;
    glGetIntegerv(GL_DEPTH_FUNC, &previousDepthFunc);
    glDepthFunc(GL_LEQUAL);

    // Обновляем буферы только если необходимо
    if (needsUpdate) {
        if (!currentTrajectory.isEmpty()) {
            QVector<QVector3D> dashedPoints;
            createDashedLine(currentTrajectory, dashedPoints);

            currentVBO.bind();
            currentVBO.allocate(dashedPoints.constData(), dashedPoints.size() * sizeof(QVector3D));
            currentVertexCount = dashedPoints.size();
        }

        if (!predictedTrajectory.isEmpty()) {
            predictedVBO.bind();
            predictedVBO.allocate(predictedTrajectory.constData(),
                                  predictedTrajectory.size() * sizeof(QVector3D));
            predictedVertexCount = predictedTrajectory.size();
        }

        needsUpdate = false;
    }

    program.setUniformValue("mvp", mvp);

    // Отрисовка текущей траектории
    if (currentVertexCount > 0) {
        currentVBO.bind();
        program.setUniformValue("color", QVector4D(1.0f, 1.0f, 1.0f, 1.0f));  // Белый цвет
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_LINES, 0, currentVertexCount);
    }

    // Отрисовка предсказанной траектории
    if (predictedVertexCount > 0) {
        predictedVBO.bind();
        program.setUniformValue("color", QVector4D(0.0f, 1.0f, 1.0f, 0.5f));  // Полупрозрачный голубой
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_LINE_STRIP, 0, predictedVertexCount);
    }

    // Восстановление состояния OpenGL
    glDepthFunc(previousDepthFunc);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);

    vao.release();
    program.release();
}

void TrajectoryRenderer::createDashedLine(const QVector<QVector3D>& sourcePoints,
                                          QVector<QVector3D>& dashedPoints)
{
    dashedPoints.clear();
    dashedPoints.reserve(sourcePoints.size() * 2);  // Примерная оценка необходимого размера

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
