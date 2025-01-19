#ifndef EARTH_RENDERER_H
#define EARTH_RENDERER_H

#include "renderer.h"
#include "tile_texture_manager.h"
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QMatrix4x4>

class EarthRenderer : public Renderer {
public:
    explicit EarthRenderer(float radius);
    ~EarthRenderer() override;

    void initialize() override;
    void render(const QMatrix4x4& projection, const QMatrix4x4& view, const QMatrix4x4& model) override;

private:
    void initShaders();
    void initTextures();
    void initGeometry();
    void createSphere(int rings, int segments);
    GLint getMaxTextureSize();

    // Константы
    static constexpr int RINGS = 180;
    static constexpr int SEGMENTS = 360;
    static constexpr float DEFAULT_DISPLACEMENT_SCALE = 0.15f;

    // Менеджеры тайлов для текстур
    TileTextureManager* earthTextureTiles;
    TileTextureManager* heightMapTiles;
    TileTextureManager* normalMapTiles;

    // Буферы OpenGL
    QOpenGLBuffer indexBuffer;
    float radius;
    int vertexCount;
    float displacementScale;

    // Текущее состояние
    QVector2D currentViewCenter;
    bool texturesInitialized;
    QVector2D calculateTextureCoordinate(const QVector3D &cameraPos);
    int calculateOptimalTileSize(int width, int height);
};

#endif // EARTH_RENDERER_H
