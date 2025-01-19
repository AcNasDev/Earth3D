#include "trajectory_renderer.h"
#include <QDateTime>

TrajectoryRenderer::TrajectoryRenderer()
    : currentVBO(QOpenGLBuffer::VertexBuffer),
    predictedVBO(QOpenGLBuffer::VertexBuffer),
    currentVertexCount(0),
    predictedVertexCount(0),
    needsUpdate(false),
    time(0.0f)
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

    vao.create();
    vao.bind();

    currentVBO.create();
    predictedVBO.create();

    vao.release();
}

void TrajectoryRenderer::initShaders()
{
    // Загружаем и компилируем шейдеры из файлов
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/trajectory.vert"))
        qDebug() << "Не удалось загрузить вертексный шейдер:" << program.log();

    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/trajectory.frag"))
        qDebug() << "Не удалось загрузить фрагментный шейдер:" << program.log();

    if (!program.link())
        qDebug() << "Не удалось слинковать шейдерную программу:" << program.log();
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
    glLineWidth(4.0f);

    GLint previousDepthFunc;
    glGetIntegerv(GL_DEPTH_FUNC, &previousDepthFunc);
    glDepthFunc(GL_LEQUAL);

    // Обновляем время для анимации
    time += 0.01f;
    if (time > 1.0f) time = 0.0f;
    program.setUniformValue("time", time);

    // Обновляем буферы только если необходимо
    if (needsUpdate) {
        if (!currentTrajectory.isEmpty()) {
            currentVBO.bind();
            currentVBO.allocate(currentTrajectory.constData(),
                                currentTrajectory.size() * sizeof(QVector3D));
            currentVertexCount = currentTrajectory.size();
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

    // Отрисовка текущей траектории (белая пунктирная линия)
    if (currentVertexCount > 0) {
        currentVBO.bind();
        program.setUniformValue("color", QVector4D(1.0f, 1.0f, 1.0f, 1.0f)); // Чисто белый цвет
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_LINE_STRIP, 0, currentVertexCount);
    }

    // Отрисовка предсказанной траектории (голубая линия)
    if (predictedVertexCount > 0) {
        predictedVBO.bind();
        program.setUniformValue("color", QVector4D(0.0f, 1.0f, 1.0f, 1.0f)); // Голубой цвет
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
