#version 330 core

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vFragPos;
flat in vec2 vTileCoord;

out vec4 fragColor;

uniform sampler2D earthTexture;
uniform sampler2D heightMap;
uniform sampler2D normalMap;
uniform vec3 lightPos;
uniform vec3 viewPos;

// Параметры освещения
uniform float ambientStrength = 0.3;
uniform float diffuseStrength = 0.7;
uniform float specularStrength = 0.05;  // Уменьшили силу бликов
uniform float shininess = 16.0;         // Уменьшили размер бликов
uniform float heightScale = 0.15;

void main() {
    // Проверяем видимость точки (если точка смотрит от нас - не рендерим её)
    vec3 viewDir = normalize(viewPos - vFragPos);
    float visibility = dot(normalize(vNormal), viewDir);

    // Если поверхность отвернута от камеры - отбрасываем фрагмент
    if (visibility < 0.0) {
        discard;
    }

    // Базовый цвет из текстуры
    vec4 texColor = texture(earthTexture, vTexCoord);

    // Получаем высоту для текущего фрагмента
    float height = texture(heightMap, vTexCoord).r;

    // Получаем нормаль из normal map
    vec3 normalFromMap = normalize(texture(normalMap, vTexCoord).rgb * 2.0 - 1.0);

    // Комбинируем геометрическую нормаль с normal map, уменьшаем влияние normal map
    vec3 N = normalize(vNormal + normalFromMap * 0.3);

    // Направление к источнику света
    vec3 lightDir = normalize(lightPos - vFragPos);

    // Ambient
    vec3 ambient = ambientStrength * texColor.rgb;

    // Diffuse с учётом рельефа
    float diff = max(dot(N, lightDir), 0.0);
    // Уменьшаем влияние высоты на освещение
    diff = diff * (1.0 + height * 0.3);
    vec3 diffuse = diff * diffuseStrength * texColor.rgb;

    // Specular с Blinn-Phong, но только для очень острых углов
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(N, halfwayDir), 0.0), shininess);
    // Добавляем затухание бликов на краях
    spec *= pow(visibility, 2.0);
    vec3 specular = specularStrength * spec * vec3(1.0);

    // Затенение в зависимости от высоты (уменьшено влияние)
    float shadowFactor = 1.0 - (height * 0.3);
    vec3 shadow = mix(vec3(1.0), vec3(shadowFactor), 0.2);

    // Комбинируем все компоненты освещения
    vec3 color = (ambient + diffuse) * shadow + specular;

    // Добавляем небольшой эффект глубины в зависимости от высоты
    color *= (1.0 + height * 0.2);

    // Добавляем затемнение по краям планеты
    color *= pow(visibility, 0.5);

    // Финальный цвет с сохранением прозрачности
    fragColor = vec4(color, texColor.a);
}
