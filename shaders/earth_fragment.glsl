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
uniform float specularStrength = 0.2;
uniform float shininess = 32.0;
uniform float heightScale = 0.15;        // Должно совпадать со значением в vertex shader

void main() {
    // Базовый цвет из текстуры
    vec4 texColor = texture(earthTexture, vTexCoord);

    // Получаем высоту для текущего фрагмента
    float height = texture(heightMap, vTexCoord).r;

    // Получаем нормаль из normal map
    vec3 normalFromMap = normalize(texture(normalMap, vTexCoord).rgb * 2.0 - 1.0);

    // Комбинируем геометрическую нормаль с normal map
    vec3 N = normalize(vNormal + normalFromMap * 0.5);

    // Направление к источнику света
    vec3 lightDir = normalize(lightPos - vFragPos);

    // Направление к камере
    vec3 viewDir = normalize(viewPos - vFragPos);

    // Ambient
    vec3 ambient = ambientStrength * texColor.rgb;

    // Diffuse с учётом рельефа
    float diff = max(dot(N, lightDir), 0.0);
    diff = diff * (1.0 + height * 0.5); // Усиливаем освещение на возвышенностях
    vec3 diffuse = diff * diffuseStrength * texColor.rgb;

    // Specular с Blinn-Phong
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(N, halfwayDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * vec3(1.0);

    // Затенение в зависимости от высоты
    float shadowFactor = 1.0 - (height * 0.5);
    vec3 shadow = mix(vec3(1.0), vec3(shadowFactor), 0.3);

    // Комбинируем все компоненты освещения
    vec3 color = (ambient + diffuse) * shadow + specular;

    // Добавляем эффект глубины в зависимости от высоты
    color *= (1.0 + height * 0.3);

    // Финальный цвет с сохранением прозрачности
    fragColor = vec4(color, texColor.a);
}
