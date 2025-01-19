#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

// Структура для информации о текстуре
struct TextureInfo {
    int tilesX;
    int tilesY;
    vec4 tilesInfo[64]; // Максимальное количество тайлов для одной текстуры
};

uniform mat4 mvp;
uniform mat4 model;
uniform mat3 normalMatrix;

// Отдельная информация для каждой текстуры
uniform TextureInfo earthTextureInfo;
uniform TextureInfo heightMapInfo;
uniform TextureInfo normalMapInfo;

out vec2 EarthTexCoord;
out vec2 HeightMapCoord;
out vec2 NormalMapCoord;
out vec3 WorldPos;
out vec3 WorldNormal;

// Функция для преобразования текстурных координат с учетом тайлов
vec2 calculateTextureCoordinates(vec2 texCoord, TextureInfo info) {
    // Определяем, в каком тайле мы находимся
    int tileX = int(texCoord.x * info.tilesX);
    int tileY = int(texCoord.y * info.tilesY);

    // Убеждаемся, что индексы в допустимых пределах
    tileX = clamp(tileX, 0, info.tilesX - 1);
    tileY = clamp(tileY, 0, info.tilesY - 1);

    // Получаем информацию о тайле
    int tileIndex = tileY * info.tilesX + tileX;
    vec4 tileInfo = info.tilesInfo[tileIndex];

    // Локальные координаты внутри тайла
    vec2 localTexCoord = vec2(
        fract(texCoord.x * info.tilesX),
        fract(texCoord.y * info.tilesY)
    );

    // Возвращаем финальные текстурные координаты
    return localTexCoord * tileInfo.zw + tileInfo.xy;
}

void main() {
    // Вычисляем текстурные координаты для каждой текстуры
    EarthTexCoord = calculateTextureCoordinates(texCoord, earthTextureInfo);
    HeightMapCoord = calculateTextureCoordinates(texCoord, heightMapInfo);
    NormalMapCoord = calculateTextureCoordinates(texCoord, normalMapInfo);

    WorldPos = vec3(model * vec4(position, 1.0));
    WorldNormal = normalMatrix * normal;
    gl_Position = mvp * vec4(position, 1.0);
}
