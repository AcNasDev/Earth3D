#ifndef SATELLITE_INFO_RENDERER_H
#define SATELLITE_INFO_RENDERER_H

#include <QPainter>
#include <QMatrix4x4>
#include "satellite.h"

class SatelliteInfoRenderer
{
public:
    SatelliteInfoRenderer();
    ~SatelliteInfoRenderer();

    void render(QPainter* painter, const QMatrix4x4& projection, const QMatrix4x4& view,
                const QMatrix4x4& model, const Satellite& satellite, const QSize& viewportSize);

private:
    QPoint worldToScreen(const QVector3D& worldPos, const QMatrix4x4& mvp, const QSize& viewportSize);
    void drawInfoBox(QPainter* painter, const QPoint& pos, const QString& info);

    static constexpr float OFFSET_Y = 20.0f; // Смещение текста от позиции спутника в пикселях
};

#endif
