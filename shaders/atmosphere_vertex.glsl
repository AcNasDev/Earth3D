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

void main() {
    // Увеличиваем радиус для атмосферы (на 2% больше радиуса Земли)
    vec3 atmospherePos = position * 1.02;

    vFragPos = vec3(modelMatrix * vec4(atmospherePos, 1.0));
    vNormal = mat3(transpose(inverse(modelMatrix))) * normal;

    // Высота относительно поверхности Земли
    vHeight = length(atmospherePos) - length(position);

    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(atmospherePos, 1.0);
}
