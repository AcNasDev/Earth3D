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
    // Базовые векторы
    vec3 viewDir = normalize(viewPos - WorldPos);
    vec3 normal = normalize(WorldNormal);

    // Направление света следует за камерой с небольшим смещением
    vec3 lightDir = normalize(viewDir + vec3(0.0, 0.3, 0.0));

    // Получаем цвет и высоту
    vec4 baseColor = texture(earthTexture, TexCoord);
    float height = texture(heightMap, TexCoord).r;

    // Базовые параметры освещения
    float ambientStrength = 0.5;
    float diffuseStrength = 0.5;

    // Вычисляем базовое освещение
    float NdotL = max(dot(normal, lightDir), 0.0);
    float NdotV = max(dot(normal, viewDir), 0.0);

    // Рассчитываем диффузное освещение с плавным переходом
    float diff = smoothstep(0.0, 1.0, NdotL);

    // Коэффициент высоты
    float heightFactor = smoothstep(0.1, 0.8, height);

    // Фактор выравнивания освещения в зависимости от угла обзора
    float viewCorrection = mix(0.7, 1.0, NdotV);

    // Рассчитываем базовое освещение
    vec3 ambient = vec3(ambientStrength);
    vec3 diffuse = vec3(diff * diffuseStrength * viewCorrection);

    // Основное освещение
    vec3 lightColor = ambient + diffuse;

    // Специальная обработка для гор
    if(height > 0.1) {
        // Увеличиваем яркость для гор с учетом угла обзора
        float mountainBrightness = mix(1.0, 1.3, heightFactor * viewCorrection);

        // Усиливаем цвет гор
        vec3 mountainColor = baseColor.rgb * mountainBrightness;

        // Применяем освещение с сохранением яркости гор
        vec3 litMountain = mountainColor * lightColor;

        // Обеспечиваем минимальную яркость для гор с учетом угла обзора
        float minBrightness = mix(0.6, 0.8, heightFactor * viewCorrection);
        litMountain = max(litMountain, mountainColor * minBrightness);

        // Финальное смешивание с учетом высоты
        FragColor = vec4(mix(baseColor.rgb * lightColor, litMountain, heightFactor), 1.0);
    } else {
        // Для низких участков используем скорректированное освещение
        vec3 litSurface = baseColor.rgb * lightColor * viewCorrection;
        float minLowBrightness = 0.5;
        litSurface = max(litSurface, baseColor.rgb * minLowBrightness);
        FragColor = vec4(litSurface, 1.0);
    }
}
