#ifndef RENDERER_H
#define RENDERER_H

#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMatrix4x4>

class Renderer : protected QOpenGLExtraFunctions
{
public:
    Renderer();
    virtual ~Renderer();

    virtual bool init();  // Новый метод для инициализации
    virtual void initialize() = 0;
    virtual void render(const QMatrix4x4& projection, const QMatrix4x4& view, const QMatrix4x4& model) = 0;

protected:
    void initializeOpenGLFunctions();

    QOpenGLShaderProgram program;
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer vbo;
};
#endif // RENDERER_H
