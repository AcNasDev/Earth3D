#version 330 core

in vec2 TexCoord;
in vec3 WorldPos;
in vec3 WorldNormal;
in float Visibility;

uniform sampler2D earthTexture;
uniform sampler2D heightMap;
uniform sampler2D normalMap;
// Добавьте эти униформы в начало fragment shader
uniform float gridThickness;
uniform vec4 gridColor;
uniform int numSegments;  // SEGMENTS
uniform int numRings;     // RINGS

out vec4 FragColor;

bool isOnGrid(vec2 uv) {
    // Вычисляем количество сегментов в UV-пространстве
    float segments = numSegments; // SEGMENTS
    float rings = numRings;    // RINGS

    // Вычисляем расстояние до ближайшей линии сетки
    float segmentSpacing = 1.0 / segments;
    float ringSpacing = 1.0 / rings;

    // Проверяем, находится ли точка рядом с линией сетки
    float segmentDist = mod(uv.x, segmentSpacing);
    float ringDist = mod(uv.y, ringSpacing);

    segmentDist = min(segmentDist, segmentSpacing - segmentDist);
    ringDist = min(ringDist, ringSpacing - ringDist);

    return segmentDist < gridThickness || ringDist < gridThickness;
}

void main()
{
    // Если фрагмент не принадлежит текущему сегменту, отбрасываем его
    if (Visibility < 0.5) {
        discard;
    }

    // Получаем цвет из текстуры
    vec4 texColor = texture(earthTexture, TexCoord);

    // Базовое освещение
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 normal = normalize(WorldNormal);

    float ambient = 0.2;
    float diffuse = max(dot(normal, lightDir), 0.0);

    // Применяем освещение к цвету текстуры
    vec3 finalColor = texColor.rgb * (ambient + diffuse);

    // Проверяем, находится ли фрагмент на линии сетки
    if (isOnGrid(TexCoord)) {
        // Смешиваем цвет текстуры с цветом сетки
        FragColor = mix(vec4(finalColor, 1.0), gridColor, gridColor.a);
    } else {
        FragColor = vec4(finalColor, 1.0);
    }
}
