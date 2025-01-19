#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 segmentIndex;

uniform mat4 mvp;
uniform mat4 model;
uniform mat3 normalMatrix;
uniform int currentRing;
uniform int currentSegment;
uniform int numRings;
uniform int numSegments;

out vec2 TexCoord;
out vec3 WorldPos;
out vec3 WorldNormal;
out float Visibility;

void main()
{
    // Преобразуем глобальные UV-координаты в локальные координаты тайла
    float tileU = texCoord.x * numSegments - currentSegment;
    float tileV = texCoord.y * numRings - currentRing;

    // Проверяем, принадлежит ли вершина текущему тайлу
    bool isCurrentTile = (int(segmentIndex.x) == currentRing &&
                         int(segmentIndex.y) == currentSegment);

    // Устанавливаем текстурные координаты для текущего тайла
    if (isCurrentTile) {
        TexCoord = vec2(
            (texCoord.x * numSegments - float(currentSegment)),
            (texCoord.y * numRings - float(currentRing))
        );
    } else {
        TexCoord = texCoord;
    }

    WorldPos = vec3(model * vec4(position, 1.0));
    WorldNormal = normalize(normalMatrix * normal);
    Visibility = float(isCurrentTile);

    gl_Position = mvp * vec4(position, 1.0);
}
