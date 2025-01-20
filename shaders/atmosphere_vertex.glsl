#version 330 core

in vec3 position;
in vec2 texCoord;
in vec3 normal;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

out vec3 vFragPos;
out vec3 vNormal;
out float vHeight;
out vec2 vTexCoord;  // Добавляем выходную переменную для текстурных координат

void main() {
    vec3 atmospherePos = position * 1.025;

    vFragPos = vec3(modelMatrix * vec4(atmospherePos, 1.0));
    vNormal = mat3(transpose(inverse(modelMatrix))) * normal;
    vHeight = length(atmospherePos) - length(position);
    vTexCoord = texCoord;  // Передаем текстурные координаты

    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(atmospherePos, 1.0);
}
