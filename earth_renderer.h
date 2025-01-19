#ifndef EARTH_RENDERER_H
#define EARTH_RENDERER_H

#include "renderer.h"
#include "tile_texture_manager.h"
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
    void createSphere();
    GLint getMaxTextureSize();

    // Константы
    static constexpr int RINGS = 8;
    static constexpr int SEGMENTS = 8;

    // Менеджеры текстур
    TileTextureManager* earthTextureTiles;
    TileTextureManager* heightMapTiles;
    TileTextureManager* normalMapTiles;

    // Буферы OpenGL
    QOpenGLBuffer indexBuffer;
    float radius;
    int vertexCount;

    struct Vertex {
        QVector3D position;
        QVector2D texCoord;
        QVector3D normal;
        QVector2D segmentIndex;  // (ring, segment)
    };

    QVector<Vertex> vertices;
    QVector<GLuint> indices;
};

#endif // EARTH_RENDERER_H
