#include "tile_texture_manager.h"
#include <QDebug>
#include <QVector2D>
#include <QImageReader>

TileTextureManager::TileTextureManager(const QString& path, int size)
    : imagePath(path)
    , tileSize(size)
    , tileCache(128)
    , textureArrayId(0)
    , isTextureArrayInitialized(false)
{
    QImageReader::setAllocationLimit(0);
}

TileTextureManager::~TileTextureManager()
{
    if (textureArrayId) {
        glDeleteTextures(1, &textureArrayId);
    }
}

void TileTextureManager::initialize()
{
    // Загружаем изображение с проверкой формата
    sourceImage.load(imagePath);
    if (sourceImage.isNull()) {
        qWarning() << "Failed to load image:" << imagePath;
        return;
    }

    // Проверяем и конвертируем формат всего изображения сразу
    if (sourceImage.format() != QImage::Format_RGBA8888) {
        qDebug() << "Converting image format from" << sourceImage.format() << "to RGBA8888";
        sourceImage = sourceImage.convertToFormat(QImage::Format_RGBA8888);
    }

    originalSize = sourceImage.size();
    tilesX = (originalSize.width() + tileSize - 1) / tileSize;
    tilesY = (originalSize.height() + tileSize - 1) / tileSize;

    qDebug() << "TileTextureManager initialized:"
             << "\nImage:" << imagePath
             << "\nSize:" << originalSize
             << "\nFormat:" << sourceImage.format()
             << "\nTiles:" << tilesX << "x" << tilesY;

    // Предварительно вычисляем информацию о тайлах
    tilesInfo.clear();
    tilesInfo.reserve(tilesX * tilesY); // Резервируем память
    for (int y = 0; y < tilesY; ++y) {
        for (int x = 0; x < tilesX; ++x) {
            tilesInfo.append(calculateTileInfo(QPoint(x, y)));
        }
    }

    // Устанавливаем размер кэша
    tileCache.setMaxCost(tilesX * tilesY);
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

    // Вычисляем размер текущего тайла
    int currentTileWidth = qMin(tileSize, originalSize.width() - x * tileSize);
    int currentTileHeight = qMin(tileSize, originalSize.height() - y * tileSize);

    QRect region(x * tileSize, y * tileSize, currentTileWidth, currentTileHeight);

    // Создаем текстуру перед копированием изображения
    QOpenGLTexture* texture = new QOpenGLTexture(QOpenGLTexture::Target2D);

    try {
        // Проверяем формат исходного изображения
        QImage::Format desiredFormat = QImage::Format_RGBA8888;
        QImage tileImage;

        if (sourceImage.format() != desiredFormat) {
            // Конвертируем только нужный регион
            tileImage = sourceImage.copy(region).convertToFormat(desiredFormat);
        } else {
            tileImage = sourceImage.copy(region);
        }

        // Проверяем успешность создания тайла
        if (tileImage.isNull()) {
            qWarning() << "Failed to create tile image at" << x << y;
            delete texture;
            return;
        }

        // Настраиваем текстуру
        texture->setFormat(QOpenGLTexture::RGBA8_UNorm);
        texture->setSize(tileImage.width(), tileImage.height());
        texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        texture->setMagnificationFilter(QOpenGLTexture::Linear);
        texture->setWrapMode(QOpenGLTexture::ClampToEdge);
        texture->setAutoMipMapGenerationEnabled(true);

        // Выделяем память и загружаем данные
        texture->allocateStorage();
        texture->setData(
            0, // уровень мипмапа
            0, // слой
            QOpenGLTexture::RGBA,
            QOpenGLTexture::UInt8,
            tileImage.constBits(),
            nullptr // настройки
            );

        // Генерируем мипмапы после загрузки данных
        texture->generateMipMaps();

        // Добавляем в кэш только если всё успешно
        tileCache.insert(tilePos, texture);

        qDebug() << "Successfully loaded tile" << tilePos
                 << "Size:" << tileImage.size()
                 << "Format:" << tileImage.format();

    } catch (const std::exception& e) {
        qWarning() << "Exception while loading tile" << tilePos << ":" << e.what();
        delete texture;
    }
}

QPoint TileTextureManager::texCoordToTile(const QVector2D& texCoord)
{
    // Преобразуем текстурные координаты [0,1] в координаты тайла
    float x = texCoord.x() * tilesX;
    float y = texCoord.y() * tilesY;

    // Возвращаем позицию тайла
    return QPoint(static_cast<int>(x), static_cast<int>(y));
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
    // Проверяем границы
    if (tilePos.x() < 0 || tilePos.x() >= tilesX ||
        tilePos.y() < 0 || tilePos.y() >= tilesY) {
        qWarning() << "Invalid tile position:" << tilePos;
        return QVector4D(0, 0, 1, 1);
    }

    // Вычисляем смещение тайла в нормализованных координатах [0,1]
    float offsetX = static_cast<float>(tilePos.x() * tileSize) / originalSize.width();
    float offsetY = static_cast<float>(tilePos.y() * tileSize) / originalSize.height();

    // Вычисляем размер тайла в нормализованных координатах
    float scaleX = static_cast<float>(tileSize) / originalSize.width();
    float scaleY = static_cast<float>(tileSize) / originalSize.height();

    // Корректируем размер для последних тайлов
    if (tilePos.x() == tilesX - 1) {
        int lastTileWidth = originalSize.width() - (tilesX - 1) * tileSize;
        scaleX = static_cast<float>(lastTileWidth) / originalSize.width();
    }
    if (tilePos.y() == tilesY - 1) {
        int lastTileHeight = originalSize.height() - (tilesY - 1) * tileSize;
        scaleY = static_cast<float>(lastTileHeight) / originalSize.height();
    }

    qDebug() << "Tile info calculated for" << tilePos
             << "\nOffset:" << offsetX << offsetY
             << "\nScale:" << scaleX << scaleY;

    return QVector4D(offsetX, offsetY, scaleX, scaleY);
}

void TileTextureManager::loadAllTiles()
{
    for (int y = 0; y < tilesY; ++y) {
        for (int x = 0; x < tilesX; ++x) {
            QPoint tilePos(x, y);
            if (!tileCache.contains(tilePos)) {
                loadTile(x, y);
            }
            tilesInfo.append(calculateTileInfo(tilePos));
        }
    }
    qDebug() << "Loaded all tiles:" << tilesX * tilesY;
}

QVector<QVector4D> TileTextureManager::getAllTilesInfo() const
{
    return tilesInfo;
}

void TileTextureManager::bindAllTiles()
{
    QMutexLocker locker(&cacheMutex);

    // Проверяем есть ли хотя бы один тайл
    if (tilesX <= 0 || tilesY <= 0) {
        qWarning() << "No tiles to bind";
        return;
    }

    // Привязываем текущий тайл (мы не должны загружать тайлы в этом методе)
    QPoint currentTilePos(0, 0);
    if (tileCache.contains(currentTilePos)) {
        QOpenGLTexture* texture = tileCache.object(currentTilePos);
        if (texture) {
            texture->bind();
        }
    }
}
