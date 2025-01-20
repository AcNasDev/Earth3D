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
    vec2 adjustedTexCoord = texCoord;
    adjustedTexCoord.x = fract(texCoord.x - time * 0.02); // Замедляем скорость анимации
    adjustedTexCoord.y = texCoord.y;
    vTexCoord = adjustedTexCoord;

    // Позиция атмосферы выше поверхности Земли
    vec3 atmospherePos = position * 1.02; // Небольшое увеличение для предотвращения z-fighting

    vFragPos = vec3(modelMatrix * vec4(atmospherePos, 1.0));
    vNormal = mat3(transpose(inverse(modelMatrix))) * normal;
    vHeight = length(atmospherePos) - length(position);

    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(atmospherePos, 1.0);
}
