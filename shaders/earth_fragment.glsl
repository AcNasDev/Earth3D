#version 330 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in mat3 TBN;

out vec4 FragColor;

uniform sampler2D earthTexture;
uniform sampler2D heightMap;
uniform sampler2D normalMap;
uniform vec3 viewPos;

// Увеличиваем значения для более заметного эффекта
const float ambientStrength = 0.2;     // Уменьшаем фоновое освещение
const float diffuseStrength = 1.0;     // Увеличиваем силу диффузного освещения
const float heightScale = 0.15;        // Увеличиваем влияние карты высот
const float normalStrength = 1.5;      // Усиливаем влияние нормалей

void main()
{
    // Получаем значение высоты и усиливаем его контраст
    float height = texture(heightMap, TexCoord).r;
    height = pow(height, 0.8); // Нелинейное усиление высот

    // Получаем и усиливаем нормаль из карты нормалей
    vec3 normalMapValue = texture(normalMap, TexCoord).rgb * 2.0 - 1.0;
    normalMapValue *= normalStrength;
    vec3 normal = normalize(TBN * normalMapValue);

    // Базовый цвет текстуры
    vec3 color = texture(earthTexture, TexCoord).rgb;

    // Направление света (от фрагмента к камере)
    vec3 lightDir = normalize(viewPos - FragPos);

    // Ambient
    vec3 ambient = ambientStrength * color;

    // Diffuse с учетом высоты
    float diff = max(dot(normal, lightDir), 0.0);
    // Усиливаем освещение на возвышенностях
    diff = diff * (1.0 + height * heightScale);
    vec3 diffuse = diffuseStrength * diff * color;

    // Добавляем затемнение в низинах
    float shadowFactor = mix(0.7, 1.0, height);

    // Комбинируем все компоненты
    vec3 finalColor = (ambient + diffuse) * shadowFactor;

    // Добавляем небольшое усиление контраста
    finalColor = pow(finalColor, vec3(1.1));

    FragColor = vec4(finalColor, 1.0);
}
