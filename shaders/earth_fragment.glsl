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
uniform vec3 lightPos = vec3(100.0, 100.0, 100.0);
uniform vec3 viewPos;
uniform float ambientStrength = 0.2;  // Увеличьте значение для лучшего освещения теневой стороны
uniform float specularStrength = 0.3; // Уменьшите для менее яркого блика
uniform float shininess = 16.0;      // Уменьшите для более мягкого блика

void main() {
    // Получаем цвет из текстуры
    vec4 texColor = texture(earthTexture, vTexCoord);
    float height = texture(heightMap, vTexCoord).r;
    vec3 normalMap = normalize(texture(normalMap, vTexCoord).rgb * 2.0 - 1.0);

    // Комбинируем нормаль из карты нормалей с геометрической нормалью
    vec3 N = normalize(vNormal + normalMap);

    // Расчет освещения
    vec3 lightDir = normalize(lightPos - vFragPos);
    vec3 viewDir = normalize(viewPos - vFragPos);
    vec3 reflectDir = reflect(-lightDir, N);

    // Ambient
    vec3 ambient = ambientStrength * texColor.rgb;

    // Diffuse
    float diff = max(dot(N, lightDir), 0.0);
    vec3 diffuse = diff * texColor.rgb;

    // Specular
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * vec3(1.0);

    // Добавляем эффект высоты
    vec3 color = (ambient + diffuse + specular);
    color *= (1.0 + height * 0.1); // Увеличиваем яркость на возвышенностях

    fragColor = vec4(color, texColor.a);
}
