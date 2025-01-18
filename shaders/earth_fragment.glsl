#version 330 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D earthTexture;
uniform vec3 viewPos;

out vec4 FragColor;

void main()
{
    vec3 lightPos = viewPos;
    vec3 lightColor = vec3(1.0);

    vec4 baseColor = texture(earthTexture, TexCoord);

    // Ambient
    vec3 ambient = 0.2 * lightColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 result = (ambient + diffuse) * baseColor.rgb;
    FragColor = vec4(result, 1.0);
}
