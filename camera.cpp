#include "camera.h"
#include <QtMath>

Camera::Camera(float radius)
    : cameraTheta(0.0f)
    , cameraPhi(M_PI_2)
    , cameraZoom(radius * 3.0f)
    , minZoom(radius * 1.5f)
    , maxZoom(radius * 10.0f)
{
    updatePosition();
}

void Camera::updatePosition()
{
    position = QVector3D(
        cameraZoom * sin(cameraPhi) * cos(cameraTheta),
        cameraZoom * cos(cameraPhi),
        cameraZoom * sin(cameraPhi) * sin(cameraTheta)
        );
}

void Camera::rotate(float deltaTheta, float deltaPhi)
{
    cameraTheta -= deltaTheta;
    cameraPhi = qBound(0.1f, cameraPhi - deltaPhi, float(M_PI - 0.1f));
    updatePosition();
}

void Camera::zoom(float factor)
{
    cameraZoom = qBound(minZoom, cameraZoom * factor, maxZoom);
    updatePosition();
}

QMatrix4x4 Camera::getViewMatrix() const
{
    QMatrix4x4 view;
    view.lookAt(position, QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    return view;
}

QVector3D Camera::getPosition() const
{
    return position;
}
