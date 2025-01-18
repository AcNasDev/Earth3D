// earth_renderer.h
#ifndef EARTH_RENDERER_H
#define EARTH_RENDERER_H

#include "renderer.h"
#include <QOpenGLTexture>
#include <QVector3D>

class EarthRenderer : public Renderer
{
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

    QOpenGLTexture* earthTexture;
    QOpenGLTexture* heightMapTexture;
    QOpenGLTexture* normalMapTexture;
    QOpenGLBuffer indexBuffer;

    float radius;
    int vertexCount;
    static constexpr int RINGS = 128;
    static constexpr int SEGMENTS = 128;
};
#endif // EARTH_RENDERER_H
