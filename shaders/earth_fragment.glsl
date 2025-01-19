#version 330 core
in vec2 TexCoord;
in vec3 WorldPos;
in vec3 WorldNormal;
in float TileLayer;

uniform sampler2DArray earthTexture;

out vec4 FragColor;

void main()
{
    // Семплируем текстуру из нужного слоя
    vec4 color = texture(earthTexture, vec3(TexCoord, TileLayer));

    // Применяем простое освещение
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 normal = normalize(WorldNormal);
    float diff = max(dot(normal, lightDir), 0.0);
    float ambient = 0.3;

    FragColor = color * (ambient + diff * 0.7);
}
