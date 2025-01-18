// earthwidget.h
#ifndef EARTHWIDGET_H
#define EARTHWIDGET_H

#include <QOpenGLWidget>
#include <QMap>
#include "camera.h"
#include "earth_renderer.h"
#include "satellite_renderer.h"
#include "trajectory_renderer.h"
#include "fps_renderer.h"
#include "satellite.h"

class EarthWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    static constexpr float EARTH_RADIUS = 6371000.0f;

    explicit EarthWidget(QWidget *parent = nullptr);
    ~EarthWidget();

    void addSatellite(int id, const QVector3D& position, const QString& info);
    void updateSatellitePosition(int id, const QVector3D& newPosition,
                                 const QVector<QVector3D>& trajectory,
                                 const QVector<QVector3D>& futureTrajectory,
                                 float angle);
    bool toggleEarthAnimation();
    bool isEarthAnimating() const { return isAnimating; }
    int getSelectedSatelliteId() const { return selectedSatelliteId; }

signals:
    void satelliteSelected(int id);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void setupSurfaceFormat();
    int pickSatellite(const QPoint& mousePos);

    // Renderers
    Camera camera;
    EarthRenderer* earthRenderer;
    SatelliteRenderer* satelliteRenderer;
    TrajectoryRenderer* trajectoryRenderer;
    FPSRenderer* fpsRenderer;

    // Matrices
    QMatrix4x4 projection;
    QMatrix4x4 model;

    // Mouse state
    QPoint lastMousePos;
    bool isMousePressed;
    bool isAnimating;

    // Satellite data
    QMap<int, Satellite> satellites;
    int selectedSatelliteId;

    // Animation
    QTimer* animationTimer;
    float rotationAngle;
};

#endif // EARTHWIDGET_H
