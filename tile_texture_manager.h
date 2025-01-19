// tile_texture_manager.h
#ifndef TILE_TEXTURE_MANAGER_H
#define TILE_TEXTURE_MANAGER_H

#include <QOpenGLFunctions>
#include <QString>
#include <QImage>
#include <QMutex>

class TileTextureManager : protected QOpenGLFunctions {
public:
    TileTextureManager(const QString& path, int size, int rings, int segments);
    ~TileTextureManager();

    void initialize();
    void loadAllTiles();
    void bindAllTiles();
    GLuint getTextureId() const { return textureArrayId; }

private:
    QString imagePath;
    int tileSize;
    QImage fullImage;
    GLuint textureArrayId;
    bool isInitialized;
    int rings;    // Количество колец сферы
    int segments; // Количество сегментов сферы
    QMutex cacheMutex;
};

#endif // TILE_TEXTURE_MANAGER_H
