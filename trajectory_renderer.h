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

    QVector<QVector3D> currentTrajectory;
    QVector<QVector3D> predictedTrajectory;

    QOpenGLBuffer currentVBO;
    QOpenGLBuffer predictedVBO;
    int currentVertexCount;
    int predictedVertexCount;
    bool needsUpdate;
    float time;  // Для анимации пунктирной линии
};

#endif // TRAJECTORY_RENDERER_H
