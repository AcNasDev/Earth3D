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
uniform float heightScale = 0.15; // Значительно увеличен масштаб высоты

void main() {
    vTexCoord = texCoord;
    vTileCoord = tileCoord;

    // Получаем значение высоты из карты высот и усиливаем его
    float height = texture(heightMap, texCoord).r;
    // Нелинейное усиление высоты для более заметного эффекта
    height = pow(height, 0.8) * 1.2;

    // Значительное смещение вершин вдоль нормали
    vec3 displacedPosition = position + normal * height * heightScale * length(position);

    // Пересчитываем нормаль с учетом рельефа
    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    vNormal = normalize(normalMatrix * normal);

    // Вычисляем позицию в мировых координатах
    vec4 worldPos = modelMatrix * vec4(displacedPosition, 1.0);
    vFragPos = worldPos.xyz;

    gl_Position = projectionMatrix * viewMatrix * worldPos;
}
