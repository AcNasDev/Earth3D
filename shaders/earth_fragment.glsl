#version 330 core

in vec2 TexCoord;
in vec3 WorldPos;
in vec3 WorldNormal;
in float Visibility;

uniform sampler2D earthTexture;
uniform float gridThickness;
uniform vec4 gridColor;
uniform int numRings;
uniform int numSegments;

out vec4 FragColor;

bool isOnGrid(vec2 uv) {
    float ringStep = 1.0 / float(numRings);
    float segmentStep = 1.0 / float(numSegments);

    vec2 gridPos = vec2(uv.x / segmentStep, uv.y / ringStep);
    vec2 gridFrac = fract(gridPos);

    return any(lessThan(gridFrac, vec2(gridThickness))) ||
           any(greaterThan(gridFrac, vec2(1.0 - gridThickness)));
}

void main()
{
    if (Visibility < 0.5) {
        discard;
    }

    // Получаем цвет из текстуры тайла
    vec4 texColor = texture(earthTexture, TexCoord);

    // Базовое освещение
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 normal = normalize(WorldNormal);
    float diffuse = max(dot(normal, lightDir), 0.0);

    vec3 finalColor = texColor.rgb * (0.2 + 0.8 * diffuse);

    // Отрисовка сетки поверх текстуры
    if (isOnGrid(TexCoord)) {
        FragColor = mix(vec4(finalColor, 1.0), gridColor, 0.5);
    } else {
        FragColor = vec4(finalColor, 1.0);
    }
}
