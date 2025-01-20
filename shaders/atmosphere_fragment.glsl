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

    // Базовый цвет атмосферы (более насыщенный голубой)
    vec3 atmosphereColor = vec3(0.5, 0.7, 1.0);

    // Эффект на краях (более заметный)
    float rim = pow(1.0 - max(0.0, dot(viewDir, normal)), 1.5);

    // Смешиваем цвета
    vec3 finalColor = mix(atmosphereColor, vec3(1.0), cloudIntensity * 0.7);

    // Добавляем свечение от солнца
    vec3 lightDir = normalize(lightPos - vFragPos);
    float sunEffect = pow(max(0.0, dot(viewDir, -lightDir)), 8.0);
    finalColor = mix(finalColor, vec3(1.0, 0.95, 0.8), sunEffect * 0.3);

    // Настраиваем прозрачность
    float alpha = (cloudIntensity * 0.6 + rim * 0.4);
    alpha = clamp(alpha, 0.2, 0.8); // Увеличиваем базовую прозрачность

    fragColor = vec4(finalColor, alpha);
}
