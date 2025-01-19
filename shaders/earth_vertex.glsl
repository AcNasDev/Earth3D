#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

uniform mat4 mvp;
uniform mat4 model;
uniform mat3 normalMatrix;
uniform int tilesX;
uniform int tilesY;

out vec2 TexCoord;
flat out int TileIndex;
out vec3 WorldPos;
out vec3 WorldNormal;

void main()
{
    // Вычисляем индекс тайла
    int tileX = int(texCoord.x * tilesX);
    int tileY = int(texCoord.y * tilesY);

    // Убеждаемся, что индексы в допустимых пределах
    tileX = clamp(tileX, 0, tilesX - 1);
    tileY = clamp(tileY, 0, tilesY - 1);

    // Вычисляем индекс тайла в текстурном массиве
    TileIndex = tileY * tilesX + tileX;

    // Вычисляем локальные текстурные координаты внутри тайла
    TexCoord = vec2(
        fract(texCoord.x * tilesX),
        fract(texCoord.y * tilesY)
    );

    WorldPos = vec3(model * vec4(position, 1.0));
    WorldNormal = normalMatrix * normal;
    gl_Position = mvp * vec4(position, 1.0);
}
