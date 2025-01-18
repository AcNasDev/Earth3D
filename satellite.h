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
    QVector<QVector3D> trajectory;
    bool isSelected;

    // Пустой конструктор
    Satellite()
        : id(-1)
        , position(QVector3D(0, 0, 0))
        , info("")
        , isSelected(false)
    {}

    // Конструктор с параметрами
    Satellite(int _id, const QVector3D& pos, const QString& _info)
        : id(_id)
        , position(pos)
        , info(_info)
        , isSelected(false)
    {}
};

#endif // SATELLITE_H
