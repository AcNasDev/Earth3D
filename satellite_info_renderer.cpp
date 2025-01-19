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
    QFont font("Arial", 10);
    painter->setFont(font);
    QFontMetrics fm(font);

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

    // Добавляем минимальные отступы
    const int padding = 4;
    maxWidth += padding * 2;
    totalHeight += padding * 2 + (lines.count() - 1) * 2;

    // Позиционируем блок справа от спутника
    int boxX = pos.x() + 5;  // Небольшой отступ вправо от точки спутника
    int boxY = pos.y() - totalHeight / 2;  // Центрируем по вертикали относительно спутника

    // Создаем прямоугольник для фона
    QRect backgroundRect(boxX, boxY, maxWidth, totalHeight);

    // Рисуем фон
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 0, 180));
    painter->drawRoundedRect(backgroundRect, 3, 3);

    // Рисуем текст
    painter->setPen(Qt::white);
    int y = backgroundRect.top() + padding;
    for (const QString& line : lines) {
        painter->drawText(boxX + padding, y + fm.ascent(), line);
        y += fm.height() + 2;
    }
}

void SatelliteInfoRenderer::render(QPainter* painter, const QMatrix4x4& projection,
                                   const QMatrix4x4& view, const QMatrix4x4& model,
                                   const Satellite& satellite, const QSize& viewportSize)
{
    QMatrix4x4 mvp = projection * view * model;
    QPoint screenPos = worldToScreen(satellite.position, mvp, viewportSize);

    if (screenPos.x() < 0 || screenPos.y() < 0 ||
        screenPos.x() > viewportSize.width() || screenPos.y() > viewportSize.height())
        return;

    // Компактный формат информации
    QString info = QString("%1\n%2").arg(satellite.id).arg(satellite.info);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::TextAntialiasing);

    drawInfoBox(painter, screenPos, info);
}
