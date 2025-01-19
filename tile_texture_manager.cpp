// tile_texture_manager.cpp
#include "tile_texture_manager.h"
#include <QImage>
#include <QtMath>
#include <QDebug>
#include <qpainter.h>

TileTextureManager::TileTextureManager(const QString& path, int rings, int segments)
    : imagePath(path)
    , numRings(rings)
    , numSegments(segments)
    , textureAtlas(nullptr)
{
    initializeOpenGLFunctions();
}

TileTextureManager::~TileTextureManager() {
}

void TileTextureManager::initialize() {
    QImage sourceImage(imagePath);
    if (sourceImage.isNull()) {
        qWarning() << "Failed to load source image:" << imagePath;
        return;
    }
    sourceImage = sourceImage.mirrored(true, false);

    // Если текстура слишком большая, разбиваем на более мелкие тайлы
    int maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);

    // Определяем размер тайла, чтобы не превысить ограничения OpenGL
    int tileWidth = std::min(sourceImage.width() / numSegments, maxTextureSize);
    int tileHeight = std::min(sourceImage.height() / numRings, maxTextureSize);

    // Создаем атлас с учетом максимального размера текстуры
    tilesPerRow = std::ceil(std::sqrt(numRings * numSegments));

    // Убедимся, что размер атласа не превышает максимально допустимый
    int atlasWidth = std::min(tileWidth * tilesPerRow, maxTextureSize);
    int atlasHeight = std::min(tileHeight * tilesPerRow, maxTextureSize);

    atlasSize = QSize(atlasWidth, atlasHeight);
    QImage atlasImage(atlasSize, QImage::Format_RGBA8888);
    atlasImage.fill(Qt::transparent);

    // Масштабируем тайлы если нужно
    float scaleX = float(atlasWidth) / (tileWidth * tilesPerRow);
    float scaleY = float(atlasHeight) / (tileHeight * tilesPerRow);

    for (int ring = 0; ring < numRings; ++ring) {
        for (int segment = 0; segment < numSegments; ++segment) {
            // Позиция в атласе
            int atlasX = (ring * numSegments + segment) % tilesPerRow * (atlasWidth / tilesPerRow);
            int atlasY = (ring * numSegments + segment) / tilesPerRow * (atlasHeight / tilesPerRow);

            // Позиция в исходной текстуре
            int sourceX = segment * (sourceImage.width() / numSegments);
            int sourceY = ring * (sourceImage.height() / numRings);

            // Размеры тайла в атласе
            int currentTileWidth = atlasWidth / tilesPerRow;
            int currentTileHeight = atlasHeight / tilesPerRow;

            // Копируем и масштабируем тайл
            QRect sourceRect(sourceX, sourceY,
                             sourceImage.width() / numSegments,
                             sourceImage.height() / numRings);
            QRect targetRect(atlasX, atlasY, currentTileWidth, currentTileHeight);

            QPainter painter(&atlasImage);
            painter.setRenderHint(QPainter::SmoothPixmapTransform);
            painter.drawImage(targetRect, sourceImage, sourceRect);

            // Сохраняем UV-координаты
            QRectF uvCoords(
                float(atlasX) / atlasSize.width(),
                float(atlasY) / atlasSize.height(),
                float(currentTileWidth) / atlasSize.width(),
                float(currentTileHeight) / atlasSize.height()
                );
            tileUVCoords.append(uvCoords);
        }
    }

    // Создаем текстуру атласа с правильными параметрами
    textureAtlas = new QOpenGLTexture(atlasImage);
    textureAtlas->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    textureAtlas->setMagnificationFilter(QOpenGLTexture::Linear);
    textureAtlas->setWrapMode(QOpenGLTexture::ClampToEdge);
    textureAtlas->generateMipMaps();
}

bool TileTextureManager::bindTileTexture(int ring, int segment) {
    if (!textureAtlas) return false;
    textureAtlas->bind();
    return true;
}

const QRectF& TileTextureManager::getTileUVCoords(int ring, int segment) {
    int index = ring * numSegments + segment;
    return tileUVCoords[index];
}

void TileTextureManager::loadTile(const TileCoords& coords) {
    QRectF uvCoords, sphereCoords;
    calculateTileCoordinates(coords, uvCoords, sphereCoords);

    // Вычисляем размеры и позицию тайла в исходном изображении
    int x = uvCoords.x() * sourceImage.width();
    int y = uvCoords.y() * sourceImage.height();
    int width = uvCoords.width() * sourceImage.width();
    int height = uvCoords.height() * sourceImage.height();

    // Вырезаем участок изображения для тайла
    QImage tileImage = sourceImage.copy(x, y, width, height);

    auto tile = new Tile();
    tile->texture = std::make_unique<QOpenGLTexture>(tileImage.mirrored());
    tile->texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    tile->texture->setMagnificationFilter(QOpenGLTexture::Linear);
    tile->texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    tile->uvCoords = uvCoords;
    tile->sphereCoords = sphereCoords;
    tile->isLoaded = true;

    // Размер в мегабайтах (приблизительно)
    int tileSizeMB = (width * height * 4) / (1024 * 1024);
    tileCache.insert(coords, tile, tileSizeMB);
}

void TileTextureManager::calculateTileCoordinates(const TileCoords& coords, QRectF& uvCoords, QRectF& sphereCoords) const {
    // UV координаты
    float u1 = static_cast<float>(coords.segment) / numSegments;
    float u2 = static_cast<float>(coords.segment + 1) / numSegments;
    float v1 = static_cast<float>(coords.ring) / numRings;
    float v2 = static_cast<float>(coords.ring + 1) / numRings;
    uvCoords = QRectF(u1, v1, u2 - u1, v2 - v1);

    // Сферические координаты
    float phi1 = v1 * M_PI;
    float phi2 = v2 * M_PI;
    float theta1 = u1 * 2.0f * M_PI;
    float theta2 = u2 * 2.0f * M_PI;
    sphereCoords = QRectF(phi1, theta1, phi2 - phi1, theta2 - theta1);
}

void TileTextureManager::updateVisibleTiles(const QMatrix4x4& viewProjection) {
    QSet<TileCoords> visibleTiles;

    // Определяем видимые тайлы
    for (int ring = 0; ring < numRings; ++ring) {
        for (int segment = 0; segment < numSegments; ++segment) {
            TileCoords coords{ring, segment};
            QRectF uvCoords, sphereCoords;
            calculateTileCoordinates(coords, uvCoords, sphereCoords);

            if (isTileVisible(sphereCoords, viewProjection)) {
                visibleTiles.insert(coords);
                // Предзагружаем тайл если его нет в кэше
                if (!tileCache.contains(coords)) {
                    loadTile(coords);
                }
            }
        }
    }

    // Удаляем невидимые тайлы из кэша
    QList<TileCoords> cachedTiles = tileCache.keys();
    for (const TileCoords& coords : cachedTiles) {
        if (!visibleTiles.contains(coords)) {
            tileCache.remove(coords);
        }
    }
}

bool TileTextureManager::isTileVisible(const QRectF& sphereCoords, const QMatrix4x4& viewProjection) const {
    // Создаем 8 угловых точек для сферического сегмента
    const float radius = 1.0f; // Единичная сфера
    QVector<QVector3D> corners;

    float phi1 = sphereCoords.x();
    float phi2 = sphereCoords.x() + sphereCoords.height();
    float theta1 = sphereCoords.y();
    float theta2 = sphereCoords.y() + sphereCoords.width();

    // Генерируем угловые точки
    for (float phi : {phi1, phi2}) {
        for (float theta : {theta1, theta2}) {
            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);
            corners.append(QVector3D(x, y, z));
        }
    }

    // Проверяем, находится ли хотя бы одна точка в пределах пирамиды видимости
    for (const QVector3D& corner : corners) {
        QVector4D clipSpace = viewProjection * QVector4D(corner, 1.0f);
        if (clipSpace.w() != 0) {
            QVector3D ndc = QVector3D(clipSpace.x(), clipSpace.y(), clipSpace.z()) / clipSpace.w();
            if (ndc.x() >= -1.0f && ndc.x() <= 1.0f &&
                ndc.y() >= -1.0f && ndc.y() <= 1.0f &&
                ndc.z() >= -1.0f && ndc.z() <= 1.0f) {
                return true;
            }
        }
    }

    return false;
}
