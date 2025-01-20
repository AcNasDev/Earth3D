#version 330 core

in vec3 vFragPos;
in vec3 vNormal;
in float vHeight;
in vec2 vTexCoord;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform float time;
uniform sampler2D skyTexture;

out vec4 fragColor;

void main() {
    vec3 viewDir = normalize(viewPos - vFragPos);
    vec3 normal = normalize(vNormal);

    // Получаем цвет облаков
    vec4 cloudColor = texture(skyTexture, vTexCoord);
    float cloudIntensity = cloudColor.r;

    // Базовый цвет атмосферы (более светлый)
    vec3 atmosphereColor = vec3(0.85, 0.9, 1.0);

    // Эффект на краях (более мягкий)
    float rim = pow(1.0 - max(0.0, dot(viewDir, normal)), 3.0);

    // Затухание на основе угла обзора
    float viewFactor = max(0.0, dot(viewDir, normal));

    // Эффект освещения от солнца
    vec3 lightDir = normalize(lightPos - vFragPos);
    float sunEffect = pow(max(0.0, dot(normal, lightDir)), 2.0);

    // Смешиваем цвета
    vec3 finalColor = mix(atmosphereColor, vec3(1.0), cloudIntensity * 0.7);
    finalColor = mix(finalColor, vec3(1.0, 0.95, 0.9), sunEffect * 0.3);

    // Настраиваем прозрачность
    float alpha = cloudIntensity * 0.5;  // Базовая прозрачность от интенсивности облаков
    alpha += rim * 0.3;                  // Добавляем эффект краёв
    alpha *= viewFactor;                 // Учитываем угол обзора

    // Усиливаем видимость облаков спереди
    if (viewFactor > 0.7) {
        alpha = mix(alpha, cloudIntensity * 0.7, (viewFactor - 0.7) / 0.3);
    }

    // Ограничиваем прозрачность
    alpha = clamp(alpha, 0.0, 0.8);

    // Отбрасываем слишком прозрачные фрагменты
    if (alpha < 0.05) {
        discard;
    }

    fragColor = vec4(finalColor, alpha);
}
