#version 330 core

in vec3 vFragPos;
in vec3 vNormal;
in float vHeight;

uniform vec3 lightPos;
uniform vec3 viewPos;

out vec4 fragColor;

void main() {
    vec3 viewDir = normalize(viewPos - vFragPos);
    float visibility = dot(normalize(vNormal), viewDir);

    // Базовый цвет атмосферы (голубоватый)
    vec3 atmosphereColor = vec3(0.5, 0.7, 1.0);

    // Рассеивание Рэлея
    float scattering = pow(max(0.0, dot(viewDir, normalize(vNormal))), 2.0);

    // Затухание по высоте
    float heightFactor = 1.0 - vHeight * 50.0;

    // Прозрачность зависит от угла обзора и высоты
    float alpha = scattering * heightFactor * 0.3;

    // Освещение от Солнца
    vec3 lightDir = normalize(lightPos - vFragPos);
    float sunEffect = pow(max(0.0, dot(viewDir, -lightDir)), 32.0);
    vec3 sunColor = vec3(1.0, 0.9, 0.7) * sunEffect;

    // Итоговый цвет
    vec3 finalColor = mix(atmosphereColor, sunColor, sunEffect);

    fragColor = vec4(finalColor, alpha);
}
