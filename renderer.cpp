#include "renderer.h"

Renderer::Renderer()
    : vbo(QOpenGLBuffer::VertexBuffer)
{
}

Renderer::~Renderer()
{
    if (vbo.isCreated())
        vbo.destroy();
    if (vao.isCreated())
        vao.destroy();
}

void Renderer::initializeOpenGLFunctions()
{
    QOpenGLExtraFunctions::initializeOpenGLFunctions();
}

bool Renderer::init()
{
    initializeOpenGLFunctions();
    return true;
}
