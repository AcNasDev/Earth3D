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

    // Вырезаем часть изображения для тайла
    QRect region = tiles[y * tilesX + x].region;
    QImage tileImage = sourceImage.copy(region);

    if (tileImage.isNull()) {
        qDebug() << "Failed to create tile image at" << x << y;
        return;
    }

    // Определяем формат текстуры на основе формата изображения
    QOpenGLTexture::TextureFormat textureFormat;
    QOpenGLTexture::PixelFormat pixelFormat;
    QOpenGLTexture::PixelType pixelType;

    switch (tileImage.format()) {
    case QImage::Format_RGB888:
        textureFormat = QOpenGLTexture::RGB8_UNorm;
        pixelFormat = QOpenGLTexture::RGB;
        pixelType = QOpenGLTexture::UInt8;
        break;
    case QImage::Format_RGBA8888:
        textureFormat = QOpenGLTexture::RGBA8_UNorm;
        pixelFormat = QOpenGLTexture::RGBA;
        pixelType = QOpenGLTexture::UInt8;
        break;
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
        tileImage = tileImage.convertToFormat(QImage::Format_RGBA8888);
        textureFormat = QOpenGLTexture::RGBA8_UNorm;
        pixelFormat = QOpenGLTexture::RGBA;
        pixelType = QOpenGLTexture::UInt8;
        break;
    default:
        // Для других форматов конвертируем в RGBA8888
        tileImage = tileImage.convertToFormat(QImage::Format_RGBA8888);
        textureFormat = QOpenGLTexture::RGBA8_UNorm;
        pixelFormat = QOpenGLTexture::RGBA;
        pixelType = QOpenGLTexture::UInt8;
        break;
    }

    QOpenGLTexture* texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    texture->setFormat(textureFormat);
    texture->setSize(tileImage.width(), tileImage.height());

    // Устанавливаем параметры фильтрации
    texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);

    // Выделяем память с учетом формата
    texture->allocateStorage();

    qDebug() << "Loading tile at" << x << y
             << "\nImage format:" << tileImage.format()
             << "\nSize:" << tileImage.size()
             << "\nBytes per line:" << tileImage.bytesPerLine()
             << "\nDepth:" << tileImage.depth()
             << "\nTexture format:" << textureFormat
             << "\nPixel format:" << pixelFormat
             << "\nPixel type:" << pixelType;

    // Загружаем данные в текстуру
    texture->setData(
        pixelFormat,
        pixelType,
        tileImage.constBits(),
        nullptr  // Опции загрузки
        );

    // Генерируем мипмапы
    texture->generateMipMaps();

    // Добавляем в кэш
    tileCache.insert(tilePos, texture);
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
    // Проверяем границы
    if (tilePos.x() < 0 || tilePos.x() >= tilesX ||
        tilePos.y() < 0 || tilePos.y() >= tilesY) {
        return QVector4D(0, 0, 1, 1); // Возвращаем дефолтные значения для невалидного тайла
    }

    // Вычисляем смещение тайла в текстурных координатах
    float offsetX = static_cast<float>(tilePos.x() * tileSize) / originalSize.width();
    float offsetY = static_cast<float>(tilePos.y() * tileSize) / originalSize.height();

    // Вычисляем масштаб тайла
    float scaleX = static_cast<float>(tileSize) / originalSize.width();
    float scaleY = static_cast<float>(tileSize) / originalSize.height();

    // Для последнего тайла в ряду/столбце корректируем масштаб
    if (tilePos.x() == tilesX - 1) {
        scaleX = static_cast<float>(originalSize.width() - tilePos.x() * tileSize) / originalSize.width();
    }
    if (tilePos.y() == tilesY - 1) {
        scaleY = static_cast<float>(originalSize.height() - tilePos.y() * tileSize) / originalSize.height();
    }

    // Возвращаем вектор с информацией о тайле:
    // xy - смещение тайла в текстурных координатах
    // zw - масштаб тайла
    return QVector4D(offsetX, offsetY, scaleX, scaleY);
}
