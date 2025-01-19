#version 330 core
in vec2 TexCoord;
in vec2 HeightMapCoord;
in vec2 NormalMapCoord;
in vec3 WorldPos;
in vec3 WorldNormal;

uniform sampler2D earthTexture;
uniform sampler2D heightMap;
uniform sampler2D normalMap;
uniform vec3 viewPos;

out vec4 FragColor;

void main()
{
    // Получаем цвет из текстуры
    vec4 color = texture(earthTexture, TexCoord);

    // Получаем нормаль из карты нормалей
    vec3 normalFromMap = normalize(texture(normalMap, NormalMapCoord).rgb * 2.0 - 1.0);

    // Комбинируем геометрическую нормаль с нормалью из карты
    vec3 finalNormal = normalize(WorldNormal + normalFromMap);

    // Базовое освещение
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(finalNormal, lightDir), 0.0);

    // Добавляем рассеянное освещение
    vec3 ambient = 0.3 * color.rgb;
    vec3 diffuse = diff * color.rgb;

    // Вычисляем направление взгляда для бликов
    vec3 viewDir = normalize(viewPos - WorldPos);
    vec3 reflectDir = reflect(-lightDir, finalNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = vec3(0.2) * spec;

    // Комбинируем все компоненты освещения
    vec3 result = ambient + diffuse + specular;

    FragColor = vec4(result, color.a);
}
