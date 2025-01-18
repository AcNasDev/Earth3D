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

const float heightScale = 0.05 * 6; // Масштаб высот

void main()
{
    // Получаем значение высоты из карты высот
    float height = texture(heightMap, TexCoord).r;

    // Получаем нормаль из карты нормалей и преобразуем ее
    vec3 normalMap = texture(normalMap, TexCoord).rgb;
    normalMap = normalMap * 2.0 - 1.0;
    vec3 normal = normalize(TBN * normalMap);

    // Базовый цвет
    vec3 color = texture(earthTexture, TexCoord).rgb;

    // Направление света (упрощенно - от камеры)
    vec3 lightDir = normalize(viewPos - FragPos);

    // Ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * color;

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * color;

    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * vec3(1.0);

    // Смешиваем все компоненты
    vec3 result = ambient + diffuse + specular;

    // Добавляем эффект высоты
    result *= (1.0 + height * heightScale);

    FragColor = vec4(result, 1.0);
}
