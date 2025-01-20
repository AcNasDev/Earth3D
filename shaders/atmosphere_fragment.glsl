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

    // Проверяем, смотрит ли фрагмент на камеру
    float facingCamera = dot(viewDir, normal);
    if (facingCamera < 0.0) {
        discard; // Отбрасываем фрагменты, смотрящие от камеры
    }

    // Получаем цвет облаков
    vec4 cloudColor = texture(skyTexture, vTexCoord);
    float cloudIntensity = cloudColor.r;

    // Базовый цвет атмосферы
    vec3 atmosphereColor = vec3(0.7, 0.85, 1.0);

    // Рассеивание на краях
    float rim = pow(1.0 - facingCamera, 2.0);

    // Высотное затухание
    float heightFactor = smoothstep(0.0, 1.0, 1.0 - vHeight * 10.0);

    // Смешиваем цвета
    vec3 finalColor = mix(atmosphereColor, vec3(1.0), cloudIntensity * 0.6);

    // Настраиваем прозрачность
    float alpha = (cloudIntensity * 0.5 + rim * 0.3) * heightFactor;
    alpha *= facingCamera; // Уменьшаем прозрачность для граней, отвернутых от камеры
    alpha = clamp(alpha, 0.0, 0.7);

    // Если прозрачность слишком низкая, отбрасываем фрагмент
    if (alpha < 0.05) {
        discard;
    }

    fragColor = vec4(finalColor, alpha);
}
