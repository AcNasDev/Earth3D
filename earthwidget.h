// earthwidget.h
#ifndef EARTHWIDGET_H
#define EARTHWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <QVector3D>
#include <QMap>
#include "satellite.h"

class EarthWidget : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
    Q_OBJECT

public:
    static constexpr float EARTH_RADIUS = 6371000.0f; // meters
    static constexpr float CAMERA_DISTANCE = EARTH_RADIUS * 3.0f;

    explicit EarthWidget(QWidget *parent = nullptr);
    ~EarthWidget();

    void addSatellite(int id, const QVector3D& position, const QString& info);
    void updateSatellitePosition(int id, const QVector3D& newPosition);
    bool  toggleEarthAnimation();
    bool isEarthAnimating() const { return isAnimating; }
    int getSelectedSatelliteId() const;
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
    void initShaders();
    void initTextures();
    void initSphereGeometry();
    void drawEarth();
    void drawSatellites();
    void drawTrajectories();
    int pickSatellite(const QPoint& mousePos);
    void drawSatellitesInfo();
    void drawSatelliteInfo(QPainter& painter, const Satellite& satellite, const QPoint& screenPos);
    QMatrix4x4 getMVPMatrix() const;

    QOpenGLShaderProgram earthProgram;
    QOpenGLShaderProgram satelliteProgram;
    QOpenGLShaderProgram lineProgram;

    QOpenGLTexture* earthTexture;

    QMatrix4x4 projection;
    QMatrix4x4 view;
    QMatrix4x4 model;

    QVector3D cameraPosition;
    float cameraTheta;
    float cameraPhi;
    float cameraZoom;

    QPoint lastMousePos;
    bool isMousePressed;
    bool isAnimating;

    QMap<int, Satellite> satellites;
    int selectedSatelliteId;

    QOpenGLBuffer sphereVBO;
    QOpenGLVertexArrayObject sphereVAO;
    int sphereVertexCount;

    QTimer* animationTimer;
    float rotationAngle;
};

#endif // EARTHWIDGET_H
