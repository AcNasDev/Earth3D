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

    // Получаем цвет из анимированной текстуры неба
    vec4 skyColor = texture(skyTexture, vTexCoord);

    // Базовый цвет атмосферы смешиваем с текстурой неба
    vec3 atmosphereColor = mix(vec3(0.5, 0.7, 1.0), skyColor.rgb, 0.5);

    float rim = pow(1.0 - max(0.0, dot(viewDir, normal)), 4.0);
    float heightFactor = smoothstep(0.0, 1.0, 1.0 - vHeight * 20.0);

    // Эффект свечения от солнца
    vec3 lightDir = normalize(lightPos - vFragPos);
    float sunEffect = pow(max(0.0, dot(viewDir, -lightDir)), 32.0);
    vec3 sunColor = vec3(1.0, 0.9, 0.7) * sunEffect;

    // Итоговый цвет
    vec3 finalColor = mix(atmosphereColor, sunColor, sunEffect);
    finalColor = mix(finalColor, vec3(1.0), rim * 0.3);

    float alpha = rim * heightFactor * 0.3;

    fragColor = vec4(finalColor, alpha);
}
