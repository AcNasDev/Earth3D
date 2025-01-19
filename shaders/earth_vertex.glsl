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
out vec3 WorldPos;
out vec3 WorldNormal;
out float TileLayer;

void main()
{
    // Вычисляем слой текстуры на основе текстурных координат
    float tileX = floor(texCoord.x * tilesX);
    float tileY = floor(texCoord.y * tilesY);
    TileLayer = tileY * tilesX + tileX;

    // Передаем локальные текстурные координаты
    TexCoord = fract(vec2(texCoord.x * tilesX, texCoord.y * tilesY));

    WorldPos = vec3(model * vec4(position, 1.0));
    WorldNormal = normalMatrix * normal;
    gl_Position = mvp * vec4(position, 1.0);
}
