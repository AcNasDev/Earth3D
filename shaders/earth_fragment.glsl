#version 330 core
in vec2 TexCoord;
in vec2 GlobalTexCoord;
in vec3 WorldPos;
in vec3 WorldNormal;

uniform sampler2DArray earthTexture;
uniform sampler2DArray heightMap;
uniform sampler2DArray normalMap;
uniform int tilesX;
uniform int tilesY;

out vec4 FragColor;

void main()
{
    // Вычисляем индексы тайлов
    int tileX = int(GlobalTexCoord.x * tilesX);
    int tileY = int(GlobalTexCoord.y * tilesY);

    // Убеждаемся, что индексы в допустимых пределах
    tileX = clamp(tileX, 0, tilesX - 1);
    tileY = clamp(tileY, 0, tilesY - 1);

    // Вычисляем индекс слоя
    int layerIndex = tileY * tilesX + tileX;

    // Получаем цвет из текущего тайла
    vec4 color = texture(earthTexture, vec3(TexCoord, layerIndex));
    vec4 heightValue = texture(heightMap, vec3(TexCoord, layerIndex));
    vec4 normalValue = texture(normalMap, vec3(TexCoord, layerIndex));

    // Применяем освещение
    vec3 normal = normalize(normalValue.rgb * 2.0 - 1.0);
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(normal, lightDir), 0.0);

    // Добавляем ambient освещение для лучшей видимости деталей
    float ambient = 0.3;
    float lighting = ambient + diff * 0.7;

    FragColor = color * lighting;
}
