#version 330 core

in vec2 TexCoord;
in vec3 WorldPos;
in vec3 WorldNormal;
in float Visibility;

uniform sampler2D earthTexture;
uniform sampler2D heightMap;
uniform sampler2D normalMap;
uniform int numSegments;
uniform int numRings;
uniform float gridThickness;
uniform vec4 gridColor;

out vec4 FragColor;

bool isOnGrid(vec2 uv) {
    // Преобразуем UV-координаты в координаты сетки
    float segmentWidth = 1.0 / float(numSegments);
    float ringHeight = 1.0 / float(numRings);

    // Вычисляем расстояние до ближайшей линии сетки
    vec2 gridPos = uv / vec2(segmentWidth, ringHeight);
    vec2 gridFrac = fract(gridPos);

    // Определяем толщину линий
    float thickness = gridThickness;

    // Проверяем близость к линиям сетки
    bool isVerticalLine = gridFrac.x < thickness || gridFrac.x > (1.0 - thickness);
    bool isHorizontalLine = gridFrac.y < thickness || gridFrac.y > (1.0 - thickness);

    return isVerticalLine || isHorizontalLine;
}

void main()
{
    // Координаты тайла должны совпадать с сеткой
    vec2 tileCoord = TexCoord;

    // Получаем цвет из текстуры
    vec4 texColor = texture(earthTexture, tileCoord);

    // Базовое освещение
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 normal = normalize(WorldNormal);
    float diffuse = max(dot(normal, lightDir), 0.0);
    vec3 ambient = vec3(0.2);
    vec3 finalColor = texColor.rgb * (ambient + diffuse);

    // Отрисовка сетки
    if (isOnGrid(tileCoord)) {
        FragColor = vec4(gridColor.rgb, gridColor.a);
    } else {
        FragColor = vec4(finalColor, 1.0);
    }
}
