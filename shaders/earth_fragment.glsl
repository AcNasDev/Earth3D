#version 330 core
in vec2 TexCoord;
flat in int TileIndex;
in vec3 WorldPos;
in vec3 WorldNormal;

uniform sampler2DArray earthTexture;
uniform sampler2DArray heightMap;
uniform sampler2DArray normalMap;

out vec4 FragColor;

void main()
{
    // Семплируем текстуры из текстурного массива
    vec4 color = texture(earthTexture, vec3(TexCoord, TileIndex));
    vec4 height = texture(heightMap, vec3(TexCoord, TileIndex));
    vec4 normalTex = texture(normalMap, vec3(TexCoord, TileIndex));

    // Применяем освещение
    vec3 normal = normalize(normalTex.rgb * 2.0 - 1.0);
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(normal, lightDir), 0.0);

    FragColor = color * (diff * 0.7 + 0.3);
}
