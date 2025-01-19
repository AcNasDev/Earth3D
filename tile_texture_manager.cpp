#include "tile_texture_manager.h"
#include <QDebug>
#include <QVector2D>
#include <QImageReader>

TileTextureManager::TileTextureManager(const QString& path, int size)
    : imagePath(path)
    , tileSize(size)
    , tileCache(12) // Кэшируем до 12 тайлов
{
    QImageReader::setAllocationLimit(0);
}

TileTextureManager::~TileTextureManager()
{
    tileCache.clear();
}

void TileTextureManager::initialize()
{
    // Загружаем изображение
    sourceImage.load(imagePath);
    if (sourceImage.isNull()) {
        qDebug() << "Failed to load image:" << imagePath;
        return;
    }

    originalSize = sourceImage.size();

    // Вычисляем количество тайлов
    tilesX = (originalSize.width() + tileSize - 1) / tileSize;
    tilesY = (originalSize.height() + tileSize - 1) / tileSize;

    qDebug() << "Initializing tile manager:"
             << "Original size:" << originalSize
             << "Tiles:" << tilesX << "x" << tilesY;

    initializeTiles();
}

void TileTextureManager::initializeTiles()
{
    tiles.clear();
    tiles.resize(tilesX * tilesY);

    for (int y = 0; y < tilesY; ++y) {
        for (int x = 0; x < tilesX; ++x) {
            Tile& tile = tiles[y * tilesX + x];
            tile.gridPos = QPoint(x, y);
            tile.region = QRect(x * tileSize, y * tileSize,
                                qMin(tileSize, originalSize.width() - x * tileSize),
                                qMin(tileSize, originalSize.height() - y * tileSize));
        }
    }
}

void TileTextureManager::loadTile(int x, int y)
{
    QMutexLocker locker(&cacheMutex);
    QPoint tilePos(x, y);

    if (tileCache.contains(tilePos)) {
        return;
    }

    // Вычисляем размер этого конкретного тайла
    int currentTileWidth = qMin(tileSize, originalSize.width() - x * tileSize);
    int currentTileHeight = qMin(tileSize, originalSize.height() - y * tileSize);

    QRect region(x * tileSize, y * tileSize, currentTileWidth, currentTileHeight);

    qDebug() << "Loading tile region:" << region
             << "for position:" << x << y;

    if (!sourceImage.isNull()) {
        QImage tileImage = sourceImage.copy(region);

        if (tileImage.isNull()) {
            qWarning() << "Failed to create tile image at" << x << y;
            return;
        }

        // Проверяем и выводим информацию о первых пикселях
        QRgb firstPixel = tileImage.pixel(0, 0);
        qDebug() << "First pixel RGBA:"
                 << qRed(firstPixel)
                 << qGreen(firstPixel)
                 << qBlue(firstPixel)
                 << qAlpha(firstPixel);

        // Конвертируем в RGBA8888 если нужно
        if (tileImage.format() != QImage::Format_RGBA8888) {
            tileImage = tileImage.convertToFormat(QImage::Format_RGBA8888);
        }

        QOpenGLTexture* texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
        texture->setFormat(QOpenGLTexture::RGBA8_UNorm);
        texture->setSize(tileImage.width(), tileImage.height());
        texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        texture->setMagnificationFilter(QOpenGLTexture::Linear);
        texture->setWrapMode(QOpenGLTexture::ClampToEdge);
        texture->allocateStorage();

        texture->setData(
            QOpenGLTexture::RGBA,
            QOpenGLTexture::UInt8,
            tileImage.constBits()
            );

        texture->generateMipMaps();
        tileCache.insert(tilePos, texture);
    } else {
        qWarning() << "Source image is null!";
    }
}

QPoint TileTextureManager::texCoordToTile(const QVector2D& texCoord)
{
    float x = texCoord.x() * originalSize.width();
    float y = texCoord.y() * originalSize.height();

    return QPoint(static_cast<int>(x) / tileSize,
                  static_cast<int>(y) / tileSize);
}

void TileTextureManager::bindTileForCoordinate(const QVector2D& texCoord)
{
    qDebug() << "Binding tile for coordinate:" << texCoord;

    // Убедимся, что координаты в правильном диапазоне
    float x = qBound(0.0f, texCoord.x(), 1.0f);
    float y = qBound(0.0f, texCoord.y(), 1.0f);

    QPoint tilePos = texCoordToTile(QVector2D(x, y));
    qDebug() << "Calculated tile position:" << tilePos;

    if (tilePos.x() >= 0 && tilePos.x() < tilesX &&
        tilePos.y() >= 0 && tilePos.y() < tilesY) {

        if (!tileCache.contains(tilePos)) {
            loadTile(tilePos.x(), tilePos.y());
        }

        QOpenGLTexture* texture = tileCache.object(tilePos);
        if (texture) {
            texture->bind();
            qDebug() << "Bound texture for tile:" << tilePos
                     << "Size:" << texture->width() << "x" << texture->height();
        }
    }
}

QVector4D TileTextureManager::getCurrentTileInfo(const QVector2D& texCoord)
{
    QPoint tilePos = texCoordToTile(texCoord);
    return calculateTileInfo(tilePos);
}

QVector4D TileTextureManager::calculateTileInfo(const QPoint& tilePos)
{
    if (tilePos.x() < 0 || tilePos.x() >= tilesX ||
        tilePos.y() < 0 || tilePos.y() >= tilesY) {
        return QVector4D(0, 0, 1, 1);
    }

    // Вычисляем размер тайла в нормализованных координатах [0,1]
    float tileWidth = static_cast<float>(tileSize) / originalSize.width();
    float tileHeight = static_cast<float>(tileSize) / originalSize.height();

    // Вычисляем смещение в нормализованных координатах
    float offsetX = tilePos.x() * tileWidth;
    float offsetY = tilePos.y() * tileHeight;

    // Для последнего тайла корректируем размер
    if (tilePos.x() == tilesX - 1) {
        tileWidth = 1.0f - offsetX;
    }
    if (tilePos.y() == tilesY - 1) {
        tileHeight = 1.0f - offsetY;
    }

    qDebug() << "Tile" << tilePos << "info:"
             << "offset:" << offsetX << offsetY
             << "scale:" << tileWidth << tileHeight;

    return QVector4D(offsetX, offsetY, tileWidth, tileHeight);
}
