#include "satellite_info_renderer.h"
#include <QFontMetrics>
#include <QDebug>
#include <QApplication>

SatelliteInfoRenderer::SatelliteInfoRenderer()
{
}

SatelliteInfoRenderer::~SatelliteInfoRenderer()
{
}

QPoint SatelliteInfoRenderer::worldToScreen(const QVector3D& worldPos, const QMatrix4x4& mvp,
                                            const QSize& viewportSize)
{
    // Преобразование из мировых координат в экранные
    QVector4D clipSpace = mvp * QVector4D(worldPos, 1.0f);

    // Проверка на видимость точки (за камерой)
    if (clipSpace.w() <= 0)
        return QPoint(-1, -1);

    clipSpace = clipSpace / clipSpace.w();

    // Преобразование в координаты экрана
    QPoint screenPos;
    screenPos.setX(int((clipSpace.x() + 1.0f) * viewportSize.width() / 2.0f));
    screenPos.setY(int((1.0f - clipSpace.y()) * viewportSize.height() / 2.0f));

    return screenPos;
}

void SatelliteInfoRenderer::drawInfoBox(QPainter* painter, const QPoint& pos, const QString& info)
{
    // Настройка шрифта
    QFontMetrics fm(qApp->font());

    // Разбиваем текст на строки
    QStringList lines = info.split('\n');

    // Вычисляем размеры текстового блока
    int maxWidth = 0;
    int totalHeight = 0;
    for (const QString& line : lines) {
        QRect bounds = fm.boundingRect(line);
        maxWidth = qMax(maxWidth, bounds.width());
        totalHeight += bounds.height();
    }

    // Добавляем отступы
    const int padding = 10;
    maxWidth += padding * 2;
    totalHeight += padding * 2 + (lines.count() - 1) * 5; // 5 пикселей между строками

    // Создаем прямоугольник для фона
    QRect backgroundRect(pos.x(), pos.y() - OFFSET_Y - totalHeight,
                         maxWidth, totalHeight);

    // Рисуем фон с закругленными углами
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 0, 180));
    painter->drawRoundedRect(backgroundRect, 5, 5);

    // Рисуем линию от спутника к информационному блоку
    painter->setPen(QPen(Qt::white, 1, Qt::DashLine));
    painter->drawLine(pos, QPoint(pos.x(), backgroundRect.bottom()));

    // Рисуем текст
    painter->setPen(Qt::white);
    int y = backgroundRect.top() + padding;
    for (const QString& line : lines) {
        painter->drawText(backgroundRect.left() + padding, y + fm.ascent(), line);
        y += fm.height() + 5;
    }
}

void SatelliteInfoRenderer::render(QPainter* painter, const QMatrix4x4& projection,
                                   const QMatrix4x4& view, const QMatrix4x4& model,
                                   const Satellite& satellite, const QSize& viewportSize)
{
    // Вычисляем MVP матрицу
    QMatrix4x4 mvp = projection * view * model;

    // Получаем экранные координаты спутника
    QPoint screenPos = worldToScreen(satellite.position, mvp, viewportSize);

    // Проверяем, находится ли спутник в поле зрения
    if (screenPos.x() < 0 || screenPos.y() < 0 ||
        screenPos.x() > viewportSize.width() || screenPos.y() > viewportSize.height())
        return;

    // Формируем текст информации
    QString info = QString("ID: %1\n%2").arg(satellite.id).arg(satellite.info);

    // Включаем сглаживание
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::TextAntialiasing);

    // Рисуем информационный блок
    drawInfoBox(painter, screenPos, info);
}
