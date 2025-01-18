// trajectory_renderer.h
#ifndef TRAJECTORY_RENDERER_H
#define TRAJECTORY_RENDERER_H

#include "renderer.h"
#include <QVector3D>

class TrajectoryRenderer : public Renderer {
public:
    TrajectoryRenderer();
    ~TrajectoryRenderer();

    void initialize() override;
    void render(const QMatrix4x4& projection, const QMatrix4x4& view, const QMatrix4x4& model) override;
    void setTrajectories(const QVector<QVector3D>& currentTrajectory,
                         const QVector<QVector3D>& predictedTrajectory);

private:
    void initShaders();
    void createDashedLine(const QVector<QVector3D>& sourcePoints, QVector<QVector3D>& dashedPoints);
    void renderTrajectory(const QVector<QVector3D>& points,
                          const QVector4D& color,
                          bool isDashed,
                          const QMatrix4x4& mvp);

    QVector<QVector3D> currentTrajectory;
    QVector<QVector3D> predictedTrajectory;
    QOpenGLBuffer vbo;
    const float dashLength = 0.1f;  // Длина штриха
    const float gapLength = 0.1f;   // Длина промежутка
};

#endif // TRAJECTORY_RENDERER_H
