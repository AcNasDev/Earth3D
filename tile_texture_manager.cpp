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
    QPoint tilePos = texCoordToTile(texCoord);

    // Проверяем границы
    if (tilePos.x() >= 0 && tilePos.x() < tilesX &&
        tilePos.y() >= 0 && tilePos.y() < tilesY) {

        // Загружаем тайл если нужно
        if (!tileCache.contains(tilePos)) {
            loadTile(tilePos.x(), tilePos.y());
        }

        // Привязываем текстуру
        QOpenGLTexture* texture = tileCache.object(tilePos);
        if (texture) {
            texture->bind();
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
    // Вычисляем смещение и масштаб для тайла
    float tileWidth = static_cast<float>(tileSize) / originalSize.width();
    float tileHeight = static_cast<float>(tileSize) / originalSize.height();

    // Позиция тайла в текстурных координатах [0,1]
    float offsetX = static_cast<float>(tilePos.x() * tileSize) / originalSize.width();
    float offsetY = static_cast<float>(tilePos.y() * tileSize) / originalSize.height();

    // Корректировка для последних тайлов
    if (tilePos.x() == tilesX - 1) {
        tileWidth = static_cast<float>(originalSize.width() % tileSize) / originalSize.width();
    }
    if (tilePos.y() == tilesY - 1) {
        tileHeight = static_cast<float>(originalSize.height() % tileSize) / originalSize.height();
    }

    qDebug() << "Tile info for pos:" << tilePos
             << "Offset:" << offsetX << offsetY
             << "Scale:" << tileWidth << tileHeight;

    return QVector4D(offsetX, offsetY, tileWidth, tileHeight);
}
