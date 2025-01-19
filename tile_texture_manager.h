#ifndef TILE_TEXTURE_MANAGER_H
#define TILE_TEXTURE_MANAGER_H

#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QMatrix4x4>
#include <QMap>
#include <QString>

struct Tile {
    GLuint textureId;
    QImage image;      // Сохраняем изображение тайла
    bool isLoaded;
    QRectF texCoords;  // UV-координаты тайла
};


class TileTextureManager : protected QOpenGLFunctions {
public:
    TileTextureManager(const QString& imagePath, int rings, int segments);
    ~TileTextureManager();

    void initialize();
    void bindTileForSegment(int ring, int segment);
    void loadTile(int ring, int segment);
    bool isTileLoaded(int ring, int segment);
private:
    void unloadTile(int ring, int segment);
    QRectF calculateTileBounds(int ring, int segment) const;
    bool isTileInViewFrustum(const QRectF& bounds, const QMatrix4x4& viewProjection) const;

    QString imagePath;
    QHash<QPair<int, int>, Tile> tiles;
    QImage sourceImage;
    int numRings;
    int numSegments;

    static constexpr int MAX_LOADED_TILES = 128;  // Максимальное количество загруженных тайлов
};

#endif // TILE_TEXTURE_MANAGER_H
