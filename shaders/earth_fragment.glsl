#version 330 core
in vec2 TexCoord;
in vec2 HeightMapCoord;
in vec2 NormalMapCoord;
in vec3 WorldPos;
in vec3 WorldNormal;

uniform sampler2D earthTexture;
uniform sampler2D heightMap;
uniform sampler2D normalMap;

out vec4 FragColor;

void main()
{
    // Получаем цвет из текстуры
    vec4 texColor = texture(earthTexture, TexCoord);

    // Базовое освещение
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 normal = normalize(WorldNormal);
    float diff = max(dot(normal, lightDir), 0.0);

    // Добавляем освещение к цвету текстуры
    vec3 ambient = 0.3 * texColor.rgb;
    vec3 diffuse = diff * texColor.rgb;
    vec3 result = ambient + diffuse;

    FragColor = vec4(result, 1.0);

    // Отладка: показываем текстурные координаты разными цветами
    // FragColor = vec4(TexCoord.x, TexCoord.y, 0.0, 1.0);
}
