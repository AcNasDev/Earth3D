#include "fps_renderer.h"

FPSRenderer::FPSRenderer()
    : frameCount(0)
    , currentFps(0.0f)
    , updateInterval(1000.0f)
{
    timer.start();
}

void FPSRenderer::update()
{
    frameCount++;

    float elapsed = timer.elapsed();
    if (elapsed >= updateInterval) {
        currentFps = frameCount * (1000.0f / elapsed);
        frameCount = 0;
        timer.restart();
    }
}

void FPSRenderer::render(QPainter& painter, const QSize& viewportSize)
{
    painter.setRenderHint(QPainter::Antialiasing);

    QFont font = painter.font();
    font.setPointSize(12);
    font.setBold(true);
    painter.setFont(font);

    QString fpsText = QString("FPS: %1").arg(QString::number(currentFps, 'f', 1));
    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(fpsText);
    textRect.adjust(-5, -5, 5, 5);
    textRect.moveTopLeft(QPoint(10, 10));

    painter.setPen(Qt::green);
    painter.fillRect(textRect, QColor(0, 0, 0, 128));
    painter.drawText(textRect, Qt::AlignCenter, fpsText);
}
