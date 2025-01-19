#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 segmentIndex;

uniform mat4 mvp;
uniform mat4 model;
uniform mat3 normalMatrix;
uniform int currentRing;    // Текущий обрабатываемый ring
uniform int currentSegment; // Текущий обрабатываемый segment

out vec2 TexCoord;
out vec3 WorldPos;
out vec3 WorldNormal;
out float Visibility;

void main()
{
    // Проверяем, принадлежит ли вершина текущему сегменту
    bool isCurrentSegment = (int(segmentIndex.x) == currentRing &&
                            int(segmentIndex.y) == currentSegment);

    WorldPos = vec3(model * vec4(position, 1.0));
    WorldNormal = normalize(normalMatrix * normal);

    // Передаем текстурные координаты без изменений
    TexCoord = texCoord;

    // Установка видимости для текущего сегмента
    Visibility = float(isCurrentSegment);

    gl_Position = mvp * vec4(position, 1.0);
}
