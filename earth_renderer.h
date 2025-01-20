// earth_renderer.h
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
    void updateVisibleTiles(const QMatrix4x4& viewProjection);

    static constexpr int RINGS = 128;     // Увеличено для лучшей детализации
    static constexpr int SEGMENTS = 128;   // Увеличено для лучшей детализации

    std::unique_ptr<TileTextureManager> earthTextureTiles;
    std::unique_ptr<TileTextureManager> heightMapTiles;
    std::unique_ptr<TileTextureManager> normalMapTiles;

    std::unique_ptr<TileTextureManager> nightLightsTiles;
    std::unique_ptr<TileTextureManager> cloudTiles;
    std::unique_ptr<TileTextureManager> specularTiles;
    std::unique_ptr<TileTextureManager> temperatureTiles;
    std::unique_ptr<TileTextureManager> snowTiles;

    float radius;

    struct Vertex {
        QVector3D position;
        QVector2D texCoord;
        QVector3D normal;
        QVector2D tileCoord;  // Координаты тайла (ring, segment)
    };

    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer vbo{QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer ibo{QOpenGLBuffer::IndexBuffer};

    QVector<Vertex> vertices;
    QVector<GLuint> indices;

    QVector3D sphericalToCartesian(float radius, float phi, float theta) const;
};

#endif // EARTH_RENDERER_H
