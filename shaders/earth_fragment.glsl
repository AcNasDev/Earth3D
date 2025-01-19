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

    vec4 texColor = texture(earthTexture, TexCoord);

    // Базовое освещение
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(normalize(WorldNormal), lightDir), 0.0);
    vec3 diffuse = diff * texColor.rgb;
    vec3 ambient = 0.3 * texColor.rgb;

    FragColor = vec4(ambient + diffuse, 1.0);
}
