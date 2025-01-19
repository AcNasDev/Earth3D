#ifndef PROGRESS_BAR_H
#define PROGRESS_BAR_H

#include "renderer.h"
#include <QOpenGLTexture>

class ProgressBar : public Renderer {
public:
    ProgressBar();
    ~ProgressBar() override;

    void initialize() override;
    void render(const QMatrix4x4& projection, const QMatrix4x4& view, const QMatrix4x4& model) override;
    void setProgress(float progress); // 0.0f to 1.0f

private:
    void initShaders();
    void initGeometry();

    float progress;
    QOpenGLBuffer indexBuffer;
};

#endif // PROGRESS_BAR_H
