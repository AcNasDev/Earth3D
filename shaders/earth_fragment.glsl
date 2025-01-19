#version 330 core

in vec2 TexCoord;
in vec3 WorldPos;
in vec3 WorldNormal;
in float Visibility;

uniform sampler2D earthTexture;

out vec4 FragColor;

void main()
{
    if (Visibility < 0.5) {
        discard;
    }

    // Сэмплируем текстуру
    vec4 texColor = texture(earthTexture, TexCoord);

    // Базовое освещение
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 normal = normalize(WorldNormal);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 ambient = texColor.rgb * 0.2;
    vec3 diffuse = texColor.rgb * diff;

    FragColor = vec4(ambient + diffuse, 1.0);
}
