#version 330 core
in vec3 fragNormal;
in vec3 fragPosition;

uniform bool isSelected;
uniform float time;

out vec4 FragColor;

void main()
{
    // Базовый цвет спутника
    vec3 baseColor = isSelected ? vec3(1.0, 0.3, 0.0) : vec3(0.9, 0.9, 1.0); // Более яркий базовый цвет

    // Направление света (фиксированное)
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));

    // Вычисляем диффузное освещение
    float diff = max(dot(fragNormal, lightDir), 0.0);

    // Добавляем ambient освещение
    float ambient = 0.5; // Увеличенное ambient освещение

    // Вычисляем specular составляющую
    vec3 viewDir = normalize(-fragPosition);
    vec3 reflectDir = reflect(-lightDir, fragNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    float specStrength = 0.8; // Увеличенная сила отражения

    // Финальный цвет
    vec3 finalColor = baseColor * (ambient + diff) + vec3(1.0) * spec * specStrength;

    // Добавляем alpha для сглаживания краёв
    float alpha = 1.0;
    if (!isSelected) {
        // Вычисляем alpha для краёв сферы
        float edgeSoftness = 1.0 - pow(1.0 - max(dot(fragNormal, viewDir), 0.0), 2.0);
        alpha = min(1.0, edgeSoftness + 0.5);
        finalColor *= 1.0 + 0.2 * sin(time * 2.0); // Добавьте uniform float time в начало шейдера

    }

    FragColor = vec4(finalColor, alpha);
}
