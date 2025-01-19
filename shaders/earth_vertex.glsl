#version 330 core

in vec3 position;
in vec2 texCoord;
in vec3 normal;
in vec2 tileCoord;

out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vFragPos;
flat out vec2 vTileCoord;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform sampler2D heightMap;
uniform float heightScale = 0.035; // Масштаб высоты рельефа

void main() {
    vTexCoord = texCoord;
    vTileCoord = tileCoord;

    // Получаем высоту из height map
    float height = texture(heightMap, texCoord).r;

    // Применяем смещение вдоль нормали
    vec3 displacedPosition = position + normal * height * heightScale;

    // Обновляем нормаль с учетом рельефа
    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    vNormal = normalize(normalMatrix * normal);

    // Вычисляем мировую позицию
    vec4 worldPos = modelMatrix * vec4(displacedPosition, 1.0);
    vFragPos = worldPos.xyz;

    gl_Position = projectionMatrix * viewMatrix * worldPos;
}
