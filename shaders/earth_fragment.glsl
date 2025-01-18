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

// Параметры освещения
const float ambientStrength = 0.3;
const float diffuseStrength = 0.7;
const float heightScale = 0.05;

void main()
{
    // Получаем значение высоты
    float height = texture(heightMap, TexCoord).r;

    // Получаем нормаль из карты нормалей
    vec3 normalMapValue = texture(normalMap, TexCoord).rgb * 2.0 - 1.0;
    vec3 normal = normalize(TBN * normalMapValue);

    // Базовый цвет текстуры
    vec3 color = texture(earthTexture, TexCoord).rgb;

    // Направление света (от фрагмента к камере)
    vec3 lightDir = normalize(viewPos - FragPos);

    // Ambient
    vec3 ambient = ambientStrength * color;

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diffuseStrength * diff * color;

    // Применяем высоту к освещению
    vec3 finalColor = (ambient + diffuse) * (1.0 + height * heightScale);

    FragColor = vec4(finalColor, 1.0);
}
