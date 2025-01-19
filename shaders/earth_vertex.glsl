#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

uniform mat4 mvp;
uniform mat4 model;
uniform mat3 normalMatrix;
uniform vec4 tilesInfo[100];  // Массив информации о тайлах
uniform int tilesX;           // Количество тайлов по X
uniform int tilesY;           // Количество тайлов по Y
uniform float displacementScale;

out vec2 TexCoord;
out vec2 HeightMapCoord;
out vec2 NormalMapCoord;
out vec3 WorldPos;
out vec3 WorldNormal;
flat out int TileIndex;

void main()
{
    // Определяем, к какому тайлу относятся текстурные координаты
    int tileX = int(texCoord.x * tilesX);
    int tileY = int(texCoord.y * tilesY);
    TileIndex = tileY * tilesX + tileX;

    // Преобразуем текстурные координаты для конкретного тайла
    vec4 tileInfo = tilesInfo[TileIndex];
    vec2 localTexCoord = fract(vec2(texCoord.x * tilesX, texCoord.y * tilesY));
    TexCoord = localTexCoord * tileInfo.zw + tileInfo.xy;

    // То же самое для карт высот и нормалей
    HeightMapCoord = localTexCoord * tileInfo.zw + tileInfo.xy;
    NormalMapCoord = localTexCoord * tileInfo.zw + tileInfo.xy;

    WorldPos = vec3(model * vec4(position, 1.0));
    WorldNormal = normalize(normalMatrix * normal);
    gl_Position = mvp * vec4(position, 1.0);
}
