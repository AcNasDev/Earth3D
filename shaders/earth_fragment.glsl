#version 330 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D earthTexture;
uniform sampler2D heightMap;
uniform sampler2D normalMap;
uniform vec3 viewPos;

out vec4 FragColor;

const float heightScale = 0.2;      // Увеличили масштаб высот
const float ambientStrength = 0.15;  // Уменьшили фоновое освещение для большего контраста
const float diffuseStrength = 1.2;   // Усилили диффузное освещение

void main()
{
    // Получаем и усиливаем высоту
    float height = texture(heightMap, TexCoord).r;
    height = pow(height, 0.75); // Нелинейное усиление контраста высот

    // Получаем и усиливаем нормаль из карты нормалей
    vec3 normalMap = texture(normalMap, TexCoord).rgb * 2.0 - 1.0;
    vec3 N = normalize(Normal + normalMap * 0.5);

    vec3 baseColor = texture(earthTexture, TexCoord).rgb;

    // Направление света (от фрагмента к камере)
    vec3 lightDir = normalize(viewPos - FragPos);

    // Ambient с учетом высоты
    vec3 ambient = ambientStrength * baseColor * (1.0 + height * 0.5);

    // Diffuse с усилением на возвышенностях
    float diff = max(dot(N, lightDir), 0.0);
    diff = diff * (1.0 + height * heightScale); // Усиливаем освещение на возвышенностях
    vec3 diffuse = diffuseStrength * diff * baseColor;

    // Затемнение в низинах
    float shadowFactor = mix(0.6, 1.0, height);

    // Добавляем цветовой оттенок для высот
    vec3 heightColor = mix(
        vec3(0.2, 0.2, 0.4),  // Цвет низин (более темный и синий)
        vec3(1.0, 0.95, 0.8),  // Цвет возвышенностей (более светлый и теплый)
        height
    );

    // Комбинируем все компоненты
    vec3 result = (ambient + diffuse) * shadowFactor;
    result *= mix(baseColor, heightColor, 0.3); // Смешиваем с цветом высот

    // Финальное усиление контраста
    result = pow(result, vec3(1.1));

    FragColor = vec4(result, 1.0);
}
