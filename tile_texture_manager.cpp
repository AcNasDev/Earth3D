#include "tile_texture_manager.h"
#include <QImage>
#include <QtMath>

TileTextureManager::TileTextureManager(const QString& path, int numRings, int numSegments)
    : imagePath(path)
    , rings(numRings)
    , segments(numSegments)
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

    // Выводим информацию об изображении
    qDebug() << "Source image loaded:"
             << "\nPath:" << imagePath
             << "\nSize:" << sourceImage.size()
             << "\nFormat:" << sourceImage.format()
             << "\nDepth:" << sourceImage.depth()
             << "\nBytes per line:" << sourceImage.bytesPerLine();

    // Проверяем, что изображение валидно после конвертации
    if (sourceImage.isNull() || !sourceImage.valid(0, 0)) {
        qDebug() << "Source image is invalid after conversion";
        return;
    }
}

void TileTextureManager::updateVisibleTiles(const QMatrix4x4& viewProjection, const QMatrix4x4& model)
{
    QSet<QPair<int, int>> newVisibleTiles;

    // Проверяем каждый сегмент на видимость
    for (int ring = 0; ring < rings; ++ring) {
        for (int segment = 0; segment < segments; ++segment) {
            QRectF bounds = calculateTileBounds(ring, segment);
            if (isTileInViewFrustum(bounds, viewProjection * model)) {
                newVisibleTiles.insert({ring, segment});

                // Загружаем тайл, если он еще не загружен
                if (!tiles.contains({ring, segment}) || !tiles[{ring, segment}].isLoaded) {
                    loadTile(ring, segment);
                }
            }
        }
    }

    // Выгружаем невидимые тайлы
    for (auto it = tiles.begin(); it != tiles.end(); ++it) {
        if (!newVisibleTiles.contains({it.key().first, it.key().second})) {
            unloadTile(it.key().first, it.key().second);
        }
    }

    visibleTiles = newVisibleTiles;
}

void TileTextureManager::loadTile(int ring, int segment)
{
    // Проверяем, не превышен ли лимит загруженных тайлов
    if (tiles.size() >= MAX_LOADED_TILES) {
        // Находим и выгружаем самый старый невидимый тайл
        for (auto it = tiles.begin(); it != tiles.end(); ++it) {
            if (!visibleTiles.contains({it.key().first, it.key().second})) {
                unloadTile(it.key().first, it.key().second);
                break;
            }
        }
    }

    // Вычисляем границы тайла в исходном изображении
    QRectF bounds = calculateTileBounds(ring, segment);
    int x = bounds.x() * sourceImage.width();
    int y = bounds.y() * sourceImage.height();
    int width = bounds.width() * sourceImage.width();
    int height = bounds.height() * sourceImage.height();

    // Вырезаем часть изображения
    QImage tileImage = sourceImage.copy(x, y, width, height);

    // Конвертируем в формат RGBA8
    tileImage = tileImage.convertToFormat(QImage::Format_RGBA8888);

    // Проверяем, что изображение валидно
    if (tileImage.isNull() || tileImage.width() <= 0 || tileImage.height() <= 0) {
        qDebug() << "Invalid tile image for ring:" << ring << "segment:" << segment
                 << "size:" << tileImage.size();
        return;
    }

    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    // Устанавливаем параметры текстуры
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Загружаем данные текстуры
    GLenum format = GL_RGBA;
    GLenum internalFormat = GL_RGBA8;

    glTexImage2D(GL_TEXTURE_2D,
                 0,                    // уровень мипмапа
                 internalFormat,       // внутренний формат
                 tileImage.width(),
                 tileImage.height(),
                 0,                    // границы
                 format,              // формат входных данных
                 GL_UNSIGNED_BYTE,    // тип входных данных
                 tileImage.constBits());

    // Проверяем ошибки OpenGL
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        qDebug() << "OpenGL error when loading tile texture:" << err
                 << "Ring:" << ring << "Segment:" << segment;
    }

    // Сохраняем информацию о тайле
    Tile tile;
    tile.ring = ring;
    tile.segment = segment;
    tile.isLoaded = true;
    tile.textureId = textureId;
    tile.bounds = bounds;

    tiles.insert({ring, segment}, tile);
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
    float u1 = static_cast<float>(segment) / segments;
    float u2 = static_cast<float>(segment + 1) / segments;
    float v1 = static_cast<float>(ring) / rings;
    float v2 = static_cast<float>(ring + 1) / rings;

    return QRectF(u1, v1, u2 - u1, v2 - v1);
}

bool TileTextureManager::isTileVisible(int ring, int segment) const
{
    // Проверяем, есть ли тайл в списке видимых
    return visibleTiles.contains({ring, segment});
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

void TileTextureManager::bindTileForSegment(int ring, int segment)
{
    auto it = tiles.find({ring, segment});
    if (it != tiles.end() && it.value().isLoaded) {
        glBindTexture(GL_TEXTURE_2D, it.value().textureId);
    }
}
