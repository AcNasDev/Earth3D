#ifndef TILE_TEXTURE_MANAGER_H
#define TILE_TEXTURE_MANAGER_H

#include <QOpenGLTexture>
#include <QImage>
#include <QVector>
#include <QCache>
#include <QMutex>

class TileTextureManager {
public:
    struct Tile {
        QOpenGLTexture* texture;
        QRect region;      // Регион в оригинальной текстуре
        QPoint gridPos;    // Позиция тайла в сетке
        bool isLoaded;

        Tile() : texture(nullptr), isLoaded(false) {}
    };

    TileTextureManager(const QString& imagePath, int tileSize = 2048);
    ~TileTextureManager();

    void initialize();
    void bindTileForCoordinate(const QVector2D& texCoord);
    QSize getOriginalSize() const { return originalSize; }
    QSize getTileSize() const { return QSize(tileSize, tileSize); }
    int getTilesX() const { return tilesX; }
    int getTilesY() const { return tilesY; }
    QVector4D getCurrentTileInfo(const QVector2D& texCoord);

private:
    QString imagePath;
    int tileSize;
    QSize originalSize;
    int tilesX, tilesY;
    QVector<Tile> tiles;
    QCache<QPoint, QOpenGLTexture> tileCache;
    QMutex cacheMutex;
    QImage sourceImage;

    void loadTile(int x, int y);
    QPoint texCoordToTile(const QVector2D& texCoord);
    void initializeTiles();
    QVector4D calculateTileInfo(const QPoint& tilePos);
};

#endif // TILE_TEXTURE_MANAGER_H
