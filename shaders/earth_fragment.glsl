#version 330 core
in vec2 TexCoord;
in vec2 HeightMapCoord;
in vec2 NormalMapCoord;
in vec3 WorldPos;
in vec3 WorldNormal;
flat in int TileIndex;

uniform sampler2D earthTexture;
uniform sampler2D heightMap;
uniform sampler2D normalMap;

out vec4 FragColor;

void main()
{
    // Получаем цвет из текстуры
    vec4 texColor = texture(earthTexture, TexCoord);
    vec4 heightMapColor = texture(heightMap, HeightMapCoord);
    vec3 normalMapColor = normalize(texture(normalMap, NormalMapCoord).rgb * 2.0 - 1.0);

    // Базовое освещение
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(normalMapColor, lightDir), 0.0);

    vec3 ambient = 0.3 * texColor.rgb;
    vec3 diffuse = diff * texColor.rgb;
    vec3 result = ambient + diffuse;

    FragColor = vec4(result, 1.0);
}
