// satellite.h
#ifndef SATELLITE_H
#define SATELLITE_H

#include <QVector3D>
#include <QString>
#include <QVector>

struct Satellite {
    int id;
    QVector3D position;
    QString info;
    QVector<QVector3D> trajectory;     // текущая траектория
    QVector<QVector3D> futureTrajectory; // будущая траектория
    float angle;        // текущий угол
    float speed;        // скорость в градусах/с
    bool isSelected;

    Satellite()
        : id(-1)
        , position(QVector3D(0, 0, 0))
        , info("")
        , angle(0.0f)
        , speed(0.0f)
        , isSelected(false)
    {}

    Satellite(int _id, const QVector3D& pos, const QString& _info, float _angle = 0.0f, float _speed = 0.0f)
        : id(_id)
        , position(pos)
        , info(_info)
        , angle(_angle)
        , speed(_speed)
        , isSelected(false)
    {}
};

#endif // SATELLITE_H
