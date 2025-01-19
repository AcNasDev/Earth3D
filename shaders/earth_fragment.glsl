#version 330 core

in vec2 TexCoord;
in vec3 WorldPos;
in vec3 WorldNormal;
in float Visibility;

uniform sampler2D earthTexture;
uniform sampler2D heightMap;
uniform sampler2D normalMap;

out vec4 FragColor;

void main()
{
    // Отбрасываем фрагменты, не принадлежащие текущему сегменту
    if (Visibility < 0.5) {
        discard;
    }

    vec4 color = texture(earthTexture, TexCoord);
    vec4 heightValue = texture(heightMap, TexCoord);
    vec3 normalValue = normalize(texture(normalMap, TexCoord).rgb * 2.0 - 1.0);

    // Базовое освещение
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 normal = normalize(WorldNormal);

    float diffuse = max(dot(normal, lightDir), 0.0);
    vec3 ambient = vec3(0.2);

    vec3 finalColor = color.rgb * (ambient + diffuse);

    FragColor = vec4(finalColor, 1.0);
}
