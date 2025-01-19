#ifndef TILE_TEXTURE_MANAGER_H
#define TILE_TEXTURE_MANAGER_H

#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QMatrix4x4>
#include <QMap>
#include <QString>

struct Tile {
    GLuint textureId;
    bool isLoaded;
    QRectF bounds;  // Границы тайла в UV-координатах
};

class TileTextureManager : protected QOpenGLFunctions {
public:
    TileTextureManager(const QString& imagePath, int rings, int segments);
    ~TileTextureManager();

    void initialize();
    void updateVisibleTiles(const QMatrix4x4& viewProjection, const QMatrix4x4& model);
    void bindTileForSegment(int ring, int segment);
    bool isTileVisible(int ring, int segment) const;
    void loadTile(int ring, int segment);
    bool isTileLoaded(int ring, int segment);
private:
    void unloadTile(int ring, int segment);
    QRectF calculateTileBounds(int ring, int segment) const;
    bool isTileInViewFrustum(const QRectF& bounds, const QMatrix4x4& viewProjection) const;

    QString imagePath;
    QImage sourceImage;
    int rings;
    int segments;
    QMap<QPair<int, int>, Tile> tiles;  // Ключ: (ring, segment)
    QSet<QPair<int, int>> visibleTiles;

    static constexpr int MAX_LOADED_TILES = 128;  // Максимальное количество загруженных тайлов
};

#endif // TILE_TEXTURE_MANAGER_H
