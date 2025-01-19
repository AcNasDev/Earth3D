#version 330 core

in vec2 TexCoord;
in vec3 WorldPos;
in vec3 WorldNormal;
in float Visibility;

uniform sampler2D earthTexture;
uniform int currentRing;
uniform int currentSegment;
uniform int numRings;
uniform int numSegments;

out vec4 FragColor;

void main()
{
    // Отбрасываем фрагменты, не принадлежащие текущему тайлу
    if (Visibility < 0.5) {
        discard;
    }

    // Нормализуем координаты тайла
    vec2 normalizedCoord = vec2(
        fract(TexCoord.x),
        fract(TexCoord.y)
    );

    // Получаем цвет из текстуры
    vec4 texColor = texture(earthTexture, normalizedCoord);

    // Базовое освещение
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 normal = normalize(WorldNormal);

    float ambient = 0.2;
    float diffuse = max(dot(normal, lightDir), 0.0);

    // Применяем освещение
    vec3 finalColor = texColor.rgb * (ambient + diffuse);

    // Добавляем отладочную окантовку тайлов
    float borderThickness = 0.02;
    if (normalizedCoord.x < borderThickness || normalizedCoord.x > (1.0 - borderThickness) ||
        normalizedCoord.y < borderThickness || normalizedCoord.y > (1.0 - borderThickness)) {
        finalColor = mix(finalColor, vec3(1.0, 1.0, 0.0), 0.5); // Желтая окантовка
    }

    FragColor = vec4(finalColor, 1.0);
}
