#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

uniform mat4 mvp;
uniform mat4 model;
uniform sampler2D heightMap;
uniform float displacementScale;
uniform vec4 tileInfo; // xy - смещение тайла, zw - масштаб тайла

out vec2 TexCoord;
out vec3 WorldNormal;
out vec3 WorldPos;

void main()
{
    // Преобразуем текстурные координаты для текущего тайла
    TexCoord = texCoord * tileInfo.zw + tileInfo.xy;

    // Применяем смещение по высоте
    float height = texture(heightMap, TexCoord).r;
    vec3 displacedPosition = position + normal * height * displacementScale;

    // Вычисляем мировые координаты
    WorldPos = vec3(model * vec4(displacedPosition, 1.0));

    // Правильно трансформируем нормаль
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    WorldNormal = normalMatrix * normal;

    // Модифицируем нормаль на основе карты высот
    if(height > 0.0) {
        vec2 texelSize = 1.0 / textureSize(heightMap, 0);
        float heightRight = texture(heightMap, TexCoord + vec2(texelSize.x, 0.0)).r;
        float heightUp = texture(heightMap, TexCoord + vec2(0.0, texelSize.y)).r;

        vec3 tangent = normalize(vec3(1.0, (heightRight - height) * displacementScale * 3.0, 0.0));
        vec3 bitangent = normalize(vec3(0.0, (heightUp - height) * displacementScale * 3.0, 1.0));
        vec3 modifiedNormal = normalize(cross(tangent, bitangent));

        WorldNormal = normalMatrix * mix(normal, modifiedNormal, height * 2.0);
    }

    gl_Position = mvp * vec4(displacedPosition, 1.0);
}
