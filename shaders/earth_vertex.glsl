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
out vec2 GlobalTexCoord;
out vec3 WorldPos;
out vec3 WorldNormal;

void main()
{
    // Сохраняем глобальные текстурные координаты
    GlobalTexCoord = texCoord;

    // Вычисляем индексы тайла
    float tileXf = texCoord.x * tilesX;
    float tileYf = texCoord.y * tilesY;

    // Используем fract для плавных переходов
    TexCoord = vec2(fract(tileXf), fract(tileYf));

    WorldPos = vec3(model * vec4(position, 1.0));
    WorldNormal = normalMatrix * normal;
    gl_Position = mvp * vec4(position, 1.0);
}
