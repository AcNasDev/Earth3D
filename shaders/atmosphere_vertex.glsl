#version 330 core

in vec3 position;
in vec2 texCoord;
in vec3 normal;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform float time;

out vec3 vFragPos;
out vec3 vNormal;
out float vHeight;
out vec2 vTexCoord;

void main() {
    // Анимация текстурных координат
    vTexCoord = texCoord;
    vTexCoord.x = fract(texCoord.x - time * 0.01); // Скорость анимации

    // Поднимаем атмосферу чуть выше поверхности
    vec3 atmospherePos = position * 1.00; // Увеличили множитель с 1.02 до 1.025

    vFragPos = vec3(modelMatrix * vec4(atmospherePos, 1.0));
    vNormal = mat3(transpose(inverse(modelMatrix))) * normal;
    vHeight = length(atmospherePos) - length(position);

    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(atmospherePos, 1.0);
}
