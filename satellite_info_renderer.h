#ifndef SATELLITE_INFO_RENDERER_H
#define SATELLITE_INFO_RENDERER_H

#include <QOpenGLFunctions_3_3_Core>  // Изменено для явного указания версии OpenGL
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QImage>
#include <QFont>
#include "satellite.h"

class SatelliteInfoRenderer : protected QOpenGLFunctions_3_3_Core  // Изменено
{
public:
    explicit SatelliteInfoRenderer();
    ~SatelliteInfoRenderer();

    bool initialize();  // Добавлен возврат bool для проверки успешности
    void render(const QMatrix4x4& projection, const QMatrix4x4& view,
                const QMatrix4x4& model, const Satellite& satellite);
    void updateInfoTexture(const Satellite& satellite);

private:
    bool initShaders();    // Изменено для возврата статуса
    bool initGeometry();   // Изменено для возврата статуса
    QImage createTextImage(const QString& text);

    QOpenGLShaderProgram program;
    QOpenGLBuffer vbo;
    QOpenGLVertexArrayObject vao;
    QOpenGLTexture* texture;
    bool isInitialized;    // Добавлено

    static constexpr float BILLBOARD_SIZE_X = 1.0f;    // Увеличенная ширина
    static constexpr float BILLBOARD_SIZE_Y = 0.5f;    // Увеличенная высота
    static constexpr float OFFSET = 0.5f;             // Увеличенное смещение от спутника
};

#endif
