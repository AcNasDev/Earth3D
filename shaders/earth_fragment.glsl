#version 330 core

in vec2 TexCoord;
in vec3 WorldNormal;
in vec3 WorldPos;

uniform sampler2D earthTexture;
uniform sampler2D heightMap;
uniform vec3 viewPos;

out vec4 FragColor;

void main()
{
    // Базовый цвет текстуры
    vec4 baseColor = texture(earthTexture, TexCoord);
    float height = texture(heightMap, TexCoord).r;

    // Нормализованные векторы
    vec3 normal = normalize(WorldNormal);
    vec3 toLight = normalize(viewPos - WorldPos);

    // Параметры освещения
    float ambientStrength = 0.3;  // Увеличили ambient для лучшей видимости
    float diffuseStrength = 0.7;
    vec3 lightColor = vec3(1.0);

    // Ambient
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse с коррекцией для вертикального угла
    float NdotL = dot(normal, toLight);
    float diff = max(NdotL, 0.0);

    // Корректируем освещение, чтобы оно было равномерным при взгляде сверху/снизу
    vec3 upVector = normalize(vec3(0.0, 1.0, 0.0));
    float verticalFactor = abs(dot(toLight, upVector));
    diff = mix(diff, max(diff, 0.5), verticalFactor);

    vec3 diffuse = diffuseStrength * diff * lightColor;

    // Добавляем эффект высоты
    float heightFactor = 1.0 + height * 0.3;

    // Комбинируем все компоненты
    vec3 result = (ambient + diffuse) * baseColor.rgb * heightFactor;

    // Добавляем мягкое затемнение на теневой стороне
    float facing = dot(normal, toLight);
    float shadowFactor = smoothstep(-0.5, 0.5, facing);
    result = mix(result * 0.4, result, shadowFactor);

    FragColor = vec4(result, 1.0);
}
