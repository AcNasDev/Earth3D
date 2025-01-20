#version 330 core

in vec3 position;
in vec2 texCoord;
in vec3 normal;

out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vFragPos;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform mat4 cloudRotationMatrix; // Добавляем матрицу вращения для облаков

void main() {
    // Применяем матрицу вращения к позиции для анимации облаков
    vec4 rotatedPosition = cloudRotationMatrix * vec4(position * 1.025, 1.0);
    vec4 worldPos = modelMatrix * rotatedPosition;

    vTexCoord = texCoord;
    vFragPos = worldPos.xyz;

    // Применяем матрицу вращения к нормалям
    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix * cloudRotationMatrix)));
    vNormal = normalize(normalMatrix * normal);

    gl_Position = projectionMatrix * viewMatrix * worldPos;
}
