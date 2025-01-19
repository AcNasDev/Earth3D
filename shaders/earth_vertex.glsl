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

out vec2 TexCoord;
out vec3 WorldPos;
out vec3 WorldNormal;
out float Visibility;

void main()
{
    // Определяем принадлежность вершины текущему тайлу
    bool isCurrentTile = (int(segmentIndex.x) == currentRing &&
                         int(segmentIndex.y) == currentSegment);

    WorldPos = vec3(model * vec4(position, 1.0));
    WorldNormal = normalize(normalMatrix * normal);
    TexCoord = texCoord;
    Visibility = float(isCurrentTile);

    gl_Position = mvp * vec4(position, 1.0);
}
