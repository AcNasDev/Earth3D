#version 330 core

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vFragPos;
flat in vec2 vTileCoord;

out vec4 fragColor;

// Базовые текстуры
uniform sampler2D earthTexture;    // Дневная текстура
uniform sampler2D heightMap;       // Карта высот
uniform sampler2D normalMap;       // Карта нормалей

// Новые текстуры
uniform sampler2D nightLightMap;   // Карта ночных огней
uniform sampler2D cloudMap;        // Карта облаков
uniform sampler2D specularMap;     // Карта бликов
uniform sampler2D temperatureMap;  // Карта температур
uniform sampler2D snowMap;         // Карта снега/льда

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float time;               // Для анимации облаков

// Параметры освещения
uniform float ambientStrength = 0.3;
uniform float diffuseStrength = 0.7;
uniform float specularStrength = 0.05;
uniform float shininess = 16.0;
uniform float heightScale = 0.15;
uniform float cloudOpacity = 0.5;  // Прозрачность облаков

void main() {
    vec3 viewDir = normalize(viewPos - vFragPos);
    float visibility = dot(normalize(vNormal), viewDir);

    if (visibility < 0.0) {
        discard;
    }

    // Базовый цвет земли
    vec4 dayColor = texture(earthTexture, vTexCoord);
    vec4 nightColor = texture(nightLightMap, vTexCoord);

    // Получаем высоту для текущего фрагмента
    float height = texture(heightMap, vTexCoord).r;

    // Облака с анимацией
    vec2 cloudUV = vTexCoord + vec2(time * 0.001, 0.0);
    vec4 clouds = texture(cloudMap, cloudUV);

    // Спекулярная карта для разных типов поверхности
    float surfaceSpecular = texture(specularMap, vTexCoord).r;

    // Температура и снег
    float temperature = texture(temperatureMap, vTexCoord).r;
    float snow = texture(snowMap, vTexCoord).r;

    // Смешиваем дневной и ночной цвет в зависимости от освещения
    vec3 lightDir = normalize(lightPos - vFragPos);
    float dayFactor = max(dot(normalize(vNormal), lightDir), 0.0);
    vec3 baseColor = mix(nightColor.rgb * 2.0, dayColor.rgb, dayFactor);

    // Добавляем снег в зависимости от высоты и температуры
    float snowFactor = clamp(snow + (height - 0.5) * 2.0 - temperature * 0.5, 0.0, 1.0);
    baseColor = mix(baseColor, vec3(0.95, 0.95, 1.0), snowFactor);

    // Освещение
    vec3 N = normalize(vNormal + normalize(texture(normalMap, vTexCoord).rgb * 2.0 - 1.0) * 0.3);

    // Ambient
    vec3 ambient = ambientStrength * baseColor;

    // Diffuse
    float diff = max(dot(N, lightDir), 0.0);
    vec3 diffuse = diff * diffuseStrength * baseColor;

    // Specular с учетом типа поверхности
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(N, halfwayDir), 0.0), shininess);
    spec *= surfaceSpecular * pow(visibility, 2.0);
    vec3 specular = specularStrength * spec * vec3(1.0);

    // Итоговый цвет
    vec3 color = ambient + diffuse + specular;

    // Добавляем облака
    color = mix(color, clouds.rgb, clouds.a * cloudOpacity * dayFactor);

    // Затемнение по краям
    color *= pow(visibility, 0.5);

    fragColor = vec4(color, 1.0);
}
