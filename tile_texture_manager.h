// tile_texture_manager.h
#ifndef TILE_TEXTURE_MANAGER_H
#define TILE_TEXTURE_MANAGER_H

#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QMatrix4x4>
#include <QCache>
#include <QString>

struct TileCoords {
    int ring;
    int segment;

    bool operator==(const TileCoords& other) const {
        return ring == other.ring && segment == other.segment;
    }
};

// For QCache
inline uint qHash(const TileCoords& key) {
    return qHash(QString("%1_%2").arg(key.ring).arg(key.segment));
}

class Tile {
public:
    Tile() : texture(nullptr), isLoaded(false) {}
    ~Tile() { }

    std::unique_ptr<QOpenGLTexture> texture;
    QRectF uvCoords;        // UV-координаты тайла в текстуре
    QRectF sphereCoords;    // Сферические координаты тайла (phi1, theta1, phi2, theta2)
    bool isLoaded;
};

class TileTextureManager : protected QOpenGLFunctions {
public:
    TileTextureManager(const QString& imagePath, int rings, int segments);
    ~TileTextureManager();

    void initialize();
    bool bindTileTexture(int ring, int segment);
    const QRectF& getTileUVCoords(int ring, int segment);
    void updateVisibleTiles(const QMatrix4x4& viewProjection);

private:
    void loadTile(const TileCoords& coords);
    void calculateTileCoordinates(const TileCoords& coords, QRectF& uvCoords, QRectF& sphereCoords) const;
    bool isTileVisible(const QRectF& sphereCoords, const QMatrix4x4& viewProjection) const;
    void cleanupUnusedTiles();

    QString imagePath;
    QImage sourceImage;
    int numRings;
    int numSegments;
    QOpenGLTexture* textureAtlas;
    QVector<QRectF> tileUVCoords;     // Кэшируем UV-координаты для каждого тайла
    QSize atlasSize;                   // Размер атласа текстур
    int tilesPerRow;                   // Количество тайлов в строке атласа

    // Используем QCache для автоматического управления памятью
    QCache<TileCoords, Tile> tileCache;
    static constexpr int MAX_CACHE_SIZE = 512; // В мегабайтах
};

#endif // TILE_TEXTURE_MANAGER_H
