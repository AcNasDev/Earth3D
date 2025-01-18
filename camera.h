#ifndef CAMERA_H
#define CAMERA_H

#include <QVector3D>
#include <QMatrix4x4>

class Camera {
public:
    Camera(float radius = 6371000.0f);

    void updatePosition();
    void rotate(float deltaTheta, float deltaPhi);
    void zoom(float factor);

    QMatrix4x4 getViewMatrix() const;
    QVector3D getPosition() const;
    float getZoom() const { return cameraZoom; }

private:
    QVector3D position;
    float cameraTheta;
    float cameraPhi;
    float cameraZoom;
    float minZoom;
    float maxZoom;
};

#endif // CAMERA_H
