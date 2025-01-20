#ifndef SATELLITE_RENDERER_H
#define SATELLITE_RENDERER_H

#include "renderer.h"
#include "satellite.h"
#include <QMap>

class SatelliteRenderer : public Renderer
{
public:
    SatelliteRenderer();
    ~SatelliteRenderer() override;

    void initialize() override;
    void render(const QMatrix4x4& projection, const QMatrix4x4& view, const QMatrix4x4& model) override;
    void updateSatellites(const QMap<int, Satellite>& satellites);

private:
    void initShaders();
    void initGeometry();
    void createSphere(int rings, int segments);

    QOpenGLBuffer indexBuffer;
    QMap<int, Satellite> satellites;
    int vertexCount;
    float time; // Добавьте эту переменную

    static constexpr int RINGS = 16;     // Меньше детализация для спутников
    static constexpr int SEGMENTS = 16;   // Меньше детализация для спутников
};

#endif // SATELLITE_RENDERER_H
