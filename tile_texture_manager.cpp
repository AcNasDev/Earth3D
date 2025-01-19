#include "tile_texture_manager.h"
#include <QImage>
#include <QtMath>

TileTextureManager::TileTextureManager(const QString& path, int _numRings, int _numSegments)
    : imagePath(path)
    , numRings(_numRings)
    , numSegments(_numSegments)
{
    initializeOpenGLFunctions();
}

TileTextureManager::~TileTextureManager()
{
    // Удаляем все загруженные текстуры
    for (const auto& tile : tiles) {
        if (tile.isLoaded) {
            glDeleteTextures(1, &tile.textureId);
        }
    }
}

void TileTextureManager::initialize()
{
    // Загружаем изображение
    sourceImage.load(imagePath);

    if (sourceImage.isNull()) {
        qDebug() << "Failed to load image:" << imagePath;
        return;
    }

    // Проверяем формат и конвертируем если нужно
    if (sourceImage.format() != QImage::Format_RGBA8888) {
        qDebug() << "Converting image from format" << sourceImage.format()
        << "to Format_RGBA8888";
        sourceImage = sourceImage.convertToFormat(QImage::Format_RGBA8888);
    }

    for (int ring = 0; ring < numRings; ++ring) {
        for (int segment = 0; segment < numSegments; ++segment) {
            loadTile(ring, segment);
        }
    }
}



bool TileTextureManager::isTileLoaded(int ring, int segment)
{
    return tiles.contains({ring, segment});
}

void TileTextureManager::loadTile(int ring, int segment)
{
    if (sourceImage.isNull()) {
        qDebug() << "Source image is null!";
        return;
    }

    // Вычисляем границы тайла в UV-координатах
    float u1 = static_cast<float>(segment) / numSegments;
    float u2 = static_cast<float>(segment + 1) / numSegments;
    float v1 = static_cast<float>(ring) / numRings;
    float v2 = static_cast<float>(ring + 1) / numRings;

    // Вычисляем границы в пикселях
    int x = u1 * sourceImage.width();
    int y = v1 * sourceImage.height();
    int width = (u2 - u1) * sourceImage.width();
    int height = (v2 - v1) * sourceImage.height();

    qDebug() << "Creating tile" << ring << segment
             << "from" << QRect(x, y, width, height);

    // Вырезаем участок изображения для тайла
    QImage tileImage = sourceImage.copy(x, y, width, height);

    // Создаем текстуру
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    // Параметры текстуры
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Загружаем данные текстуры
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 tileImage.width(), tileImage.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, tileImage.constBits());

    // Сохраняем информацию о тайле
    Tile tile;
    tile.textureId = textureId;
    tile.image = tileImage;
    tile.isLoaded = true;
    tile.texCoords = QRectF(u1, v1, u2 - u1, v2 - v1);

    tiles.insert({ring, segment}, tile);

    qDebug() << "Tile" << ring << segment << "created with ID" << textureId;
}

void TileTextureManager::unloadTile(int ring, int segment)
{
    auto it = tiles.find({ring, segment});
    if (it != tiles.end() && it.value().isLoaded) {
        glDeleteTextures(1, &it.value().textureId);
        tiles.remove({ring, segment});
    }
}

QRectF TileTextureManager::calculateTileBounds(int ring, int segment) const
{
    // Вычисляем UV-координаты тайла
    float u1 = static_cast<float>(segment) / segment;
    float u2 = static_cast<float>(segment + 1) / segment;
    float v1 = static_cast<float>(ring) / ring;
    float v2 = static_cast<float>(ring + 1) / ring;

    return QRectF(QPointF(u1, v1), QPointF(u2, v2));
}

bool TileTextureManager::isTileInViewFrustum(const QRectF& bounds, const QMatrix4x4& viewProjection) const
{
    // Создаем 8 угловых точек для сферического сегмента
    QVector<QVector3D> corners;

    // Конвертируем UV координаты в сферические координаты
    float phi1 = bounds.top() * M_PI;        // Начальная широта
    float phi2 = bounds.bottom() * M_PI;     // Конечная широта
    float theta1 = bounds.left() * 2 * M_PI; // Начальная долгота
    float theta2 = bounds.right() * 2 * M_PI;// Конечная долгота

    // Генерируем 8 угловых точек сегмента сферы
    for (float phi : {phi1, phi2}) {
        for (float theta : {theta1, theta2}) {
            // Внутренняя точка (на поверхности сферы)
            float x = sin(phi) * cos(theta);
            float y = cos(phi);
            float z = sin(phi) * sin(theta);
            corners.append(QVector3D(x, y, z));

            // Внешняя точка (с учетом высоты рельефа)
            const float heightScale = 0.1f; // Максимальная высота рельефа (10% от радиуса)
            corners.append(QVector3D(x * (1.0f + heightScale),
                                     y * (1.0f + heightScale),
                                     z * (1.0f + heightScale)));
        }
    }

    // Проверяем, находится ли хотя бы одна точка в видимой области
    for (const auto& corner : corners) {
        QVector4D clipSpace = viewProjection * QVector4D(corner, 1.0f);

        // Проверка на w == 0 для предотвращения деления на ноль
        if (qFuzzyIsNull(clipSpace.w())) {
            continue;
        }

        // Нормализация в clip space
        clipSpace /= clipSpace.w();

        // Если хотя бы одна точка находится в видимом объеме ([-1,1] для всех координат),
        // считаем тайл видимым
        if (clipSpace.x() >= -1.0f && clipSpace.x() <= 1.0f &&
            clipSpace.y() >= -1.0f && clipSpace.y() <= 1.0f &&
            clipSpace.z() >= -1.0f && clipSpace.z() <= 1.0f) {
            return true;
        }
    }

    // Дополнительная проверка для случая, когда камера находится внутри сегмента
    QVector3D cameraPos = viewProjection.inverted().column(3).toVector3D();
    QVector3D segmentCenter = QVector3D(
        sin((phi1 + phi2) * 0.5f) * cos((theta1 + theta2) * 0.5f),
        cos((phi1 + phi2) * 0.5f),
        sin((phi1 + phi2) * 0.5f) * sin((theta1 + theta2) * 0.5f)
        );

    float distanceToCamera = (cameraPos - segmentCenter).length();
    if (distanceToCamera < 1.2f) { // Немного больше радиуса сферы для учета высоты рельефа
        return true;
    }

    return false;
}

bool TileTextureManager::bindTileForSegment(int ring, int segment)
{
    auto it = tiles.find({ring, segment});
    if (it != tiles.end() && it.value().isLoaded) {
        glBindTexture(GL_TEXTURE_2D, it.value().textureId);
        return true;
    }
    return false;
}
