#version 330 core

in vec3 vFragPos;
in vec3 vNormal;
in float vHeight;
in vec2 vTexCoord;  // Добавляем текстурные координаты

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float time;
uniform sampler2D skyTexture;  // Добавляем текстуру неба

out vec4 fragColor;

void main() {
    vec3 viewDir = normalize(viewPos - vFragPos);
    vec3 lightDir = normalize(lightPos - vFragPos);

    // Получаем цвет из текстуры неба
    vec4 skyColor = texture(skyTexture, vTexCoord);

    // Базовый цвет атмосферы
    vec3 atmosphereColor = mix(vec3(0.5, 0.7, 1.0), skyColor.rgb, 0.5);

    // Рассеивание света
    float scattering = pow(max(0.0, dot(viewDir, normalize(vNormal))), 2.0);

    // Затухание по высоте
    float heightFactor = smoothstep(0.0, 1.0, 1.0 - vHeight * 20.0);

    // Свечение на краях
    float rimEffect = pow(1.0 - max(0.0, dot(viewDir, normalize(vNormal))), 4.0);

    // Эффект солнца
    float sunEffect = pow(max(0.0, dot(viewDir, -lightDir)), 32.0);
    vec3 sunColor = vec3(1.0, 0.9, 0.7) * sunEffect;

    // Итоговый цвет
    vec3 finalColor = mix(atmosphereColor, sunColor, sunEffect);
    finalColor = mix(finalColor, vec3(1.0), rimEffect * 0.3);

    // Прозрачность
    float alpha = scattering * heightFactor * 0.3;

    fragColor = vec4(finalColor, alpha);
}
