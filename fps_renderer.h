#ifndef FPS_RENDERER_H
#define FPS_RENDERER_H

#include <QElapsedTimer>
#include <QPainter>

class FPSRenderer {
public:
    FPSRenderer();
    void update();
    void render(QPainter& painter, const QSize& viewportSize);

private:
    QElapsedTimer timer;
    int frameCount;
    float currentFps;
    const float updateInterval;
};

#endif // FPS_RENDERER_H
