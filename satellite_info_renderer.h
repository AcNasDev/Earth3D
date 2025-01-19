#ifndef SATELLITE_INFO_RENDERER_H
#define SATELLITE_INFO_RENDERER_H

#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QImage>
#include <QFont>
#include "satellite.h"

class SatelliteInfoRenderer : protected QOpenGLFunctions
{
public:
    SatelliteInfoRenderer();
    ~SatelliteInfoRenderer();

    void initialize();
    void render(const QMatrix4x4& projection, const QMatrix4x4& view,
                const QMatrix4x4& model, const Satellite& satellite);
    void updateInfoTexture(const Satellite& satellite);

private:
    void initShaders();
    void initGeometry();
    QImage createTextImage(const QString& text);

    QOpenGLShaderProgram program;
    QOpenGLBuffer vbo;
    QOpenGLVertexArrayObject vao;
    QOpenGLTexture* texture;

    static constexpr float BILLBOARD_SIZE = 0.2f; // Размер информационной панели
    static constexpr float OFFSET = 0.3f;         // Смещение от позиции спутника
};

#endif // SATELLITE_INFO_RENDERER_H
