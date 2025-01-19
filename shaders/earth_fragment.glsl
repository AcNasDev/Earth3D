#version 330 core

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vFragPos;
flat in vec2 vTileCoord;

out vec4 fragColor;

uniform sampler2D earthTexture;
uniform sampler2D heightMap;
uniform sampler2D normalMap;

// Параметры освещения
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float ambientStrength = 0.6;
uniform float specularStrength = 0.2;
uniform float shininess = 8.0;
uniform float atmosphereStrength = 0.15;

void main() {
    // Получаем цвет из текстуры
    vec4 texColor = texture(earthTexture, vTexCoord);
    float height = texture(heightMap, vTexCoord).r;
    vec3 normalMap = normalize(texture(normalMap, vTexCoord).rgb * 2.0 - 1.0);

    // Усиливаем влияние карты нормалей для более выраженного рельефа
    vec3 N = normalize(vNormal + normalMap * 0.8);

    // Расчет освещения с учетом расстояния
    vec3 lightDir = normalize(lightPos - vFragPos);
    vec3 viewDir = normalize(viewPos - vFragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    // Расчет затухания света
    float distance = length(lightPos - vFragPos);
    float attenuation = 1.0 / (1.0 + 0.000000001 * distance * distance);

    // Ambient с учетом атмосферного рассеивания
    float NdotV = max(dot(N, viewDir), 0.0);
    float rimEffect = 1.0 - NdotV;
    vec3 ambient = (ambientStrength + rimEffect * atmosphereStrength) * texColor.rgb;

    // Diffuse с плавным переходом и усилением теней
    float diff = max(dot(N, lightDir), 0.0);
    diff = smoothstep(0.0, 1.0, diff);
    vec3 diffuse = diff * texColor.rgb;

    // Specular с использованием Blinn-Phong
    float spec = pow(max(dot(N, halfwayDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * vec3(1.0);

    // Применяем затухание к diffuse и specular составляющим
    diffuse *= attenuation;
    specular *= attenuation;

    // Усиливаем эффект высоты для более выраженного рельефа
    vec3 color = ambient + diffuse + specular;
    color *= (1.0 + height * 0.15); // Увеличено влияние высоты

    // Добавляем эффект затенения в низинах
    color *= (0.85 + height * 0.3);

    // Добавляем мягкое насыщение цвета
    color = color / (color + vec3(1.0));

    fragColor = vec4(color, texColor.a);
}
