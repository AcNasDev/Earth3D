#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 segmentIndex;

uniform mat4 mvp;
uniform mat4 model;
uniform mat3 normalMatrix;
uniform mat4 viewProjection;
uniform int currentRing;
uniform int currentSegment;

out vec2 TexCoord;
out vec3 WorldPos;
out vec3 WorldNormal;
out float Visibility;

void main()
{
    // Проверяем, принадлежит ли вершина текущему сегменту
    float isCurrentSegment = float(int(segmentIndex.x) == currentRing &&
                                 int(segmentIndex.y) == currentSegment);

    WorldPos = vec3(model * vec4(position, 1.0));
    WorldNormal = normalMatrix * normal;
    TexCoord = texCoord;

    gl_Position = mvp * vec4(position, 1.0);

    // Если вершина не принадлежит текущему сегменту, делаем её невидимой
    Visibility = isCurrentSegment;
}
