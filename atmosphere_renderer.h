#ifndef ATMOSPHERE_RENDERER_H
#define ATMOSPHERE_RENDERER_H

#include "renderer.h"
#include "tile_texture_manager.h"

class AtmosphereRenderer : public Renderer {
public:
    explicit AtmosphereRenderer(float earthRadius);
    ~AtmosphereRenderer() override;

    void initialize() override;
    void render(const QMatrix4x4& projection, const QMatrix4x4& view, const QMatrix4x4& model) override;


protected:
    void initShaders();
    void initGeometry();
    void initTextures();  // Добавляем метод для инициализации текстур

private:
    QMatrix4x4 cloudRotationMatrix;
    float rotationAngle = 0.0f;
    void createSphere();
    QVector3D sphericalToCartesian(float radius, float phi, float theta) const;
    void update(float deltaTime);

    float radius;

    QOpenGLBuffer ibo{QOpenGLBuffer::IndexBuffer};

    // Добавляем текстуру неба
    std::unique_ptr<TileTextureManager> skyTexture;

    struct Vertex {
        QVector3D position;
        QVector2D texCoord;
        QVector3D normal;
    };

    QVector<Vertex> vertices;
    QVector<GLuint> indices;
    const int RINGS = 32;
    const int SEGMENTS = 32;
};

#endif // ATMOSPHERE_RENDERER_H
