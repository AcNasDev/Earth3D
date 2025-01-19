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
out vec2 GlobalTexCoord; // Добавляем для определения тайла
out vec3 WorldPos;
out vec3 WorldNormal;

void main()
{
    // Сохраняем оригинальные текстурные координаты для определения тайла
    GlobalTexCoord = texCoord;

    // Вычисляем локальные текстурные координаты внутри тайла
    float tileU = fract(texCoord.x * tilesX);
    float tileV = fract(texCoord.y * tilesY);
    TexCoord = vec2(tileU, tileV);

    WorldPos = vec3(model * vec4(position, 1.0));
    WorldNormal = normalMatrix * normal;
    gl_Position = mvp * vec4(position, 1.0);
}
