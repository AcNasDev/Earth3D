// tile_texture_manager.cpp
#include "tile_texture_manager.h"
#include <QDebug>
#include <QImageReader>

TileTextureManager::TileTextureManager(const QString& path, int size, int sphereRings, int sphereSegments)
    : imagePath(path)
    , tileSize(size)
    , textureArrayId(0)
    , isInitialized(false)
    , rings(sphereRings)
    , segments(sphereSegments)
{
    initializeOpenGLFunctions();
}

TileTextureManager::~TileTextureManager()
{
    if (textureArrayId) {
        glDeleteTextures(1, &textureArrayId);
    }
}

void TileTextureManager::initialize()
{
    if (isInitialized) return;

    // Загружаем полное изображение
    fullImage = QImage(imagePath);
    if (fullImage.isNull()) {
        qWarning() << "Failed to load image:" << imagePath;
        return;
    }

    // Конвертируем в RGBA формат если нужно
    if (fullImage.format() != QImage::Format_RGBA8888) {
        fullImage = fullImage.convertToFormat(QImage::Format_RGBA8888);
    }

    // Создаем текстурный массив
    glGenTextures(1, &textureArrayId);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayId);

    // Создаем хранилище для всех сегментов
    int totalLayers = rings * segments;
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8,
                 tileSize, tileSize, totalLayers,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Настраиваем параметры текстуры
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    isInitialized = true;
}

void TileTextureManager::loadAllTiles()
{
    QMutexLocker locker(&cacheMutex);

    for (int ring = 0; ring < rings; ++ring) {
        float v1 = static_cast<float>(ring) / rings;
        float v2 = static_cast<float>(ring + 1) / rings;

        for (int seg = 0; seg < segments; ++seg) {
            float u1 = 1.0f - static_cast<float>(seg + 1) / segments;
            float u2 = 1.0f - static_cast<float>(seg) / segments;

            // Вычисляем пиксельные координаты в исходном изображении
            int startX = static_cast<int>(u1 * fullImage.width());
            int endX = static_cast<int>(u2 * fullImage.width());
            int startY = static_cast<int>(v1 * fullImage.height());
            int endY = static_cast<int>(v2 * fullImage.height());

            // Вырезаем и масштабируем изображение для текущего сегмента
            QImage segmentImage = fullImage.copy(
                                               startX, startY,
                                               endX - startX,
                                               endY - startY
                                               ).scaled(
                                          tileSize, tileSize,
                                          Qt::IgnoreAspectRatio,
                                          Qt::SmoothTransformation
                                          );

            // Загружаем в текстурный массив
            int layer = ring * segments + seg;
            glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayId);
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                            0, 0, layer,
                            tileSize, tileSize, 1,
                            GL_RGBA, GL_UNSIGNED_BYTE,
                            segmentImage.constBits());
        }
    }
}

void TileTextureManager::bindAllTiles()
{
    QMutexLocker locker(&cacheMutex);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayId);
}
