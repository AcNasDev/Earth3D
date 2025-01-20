#version 330 core

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vFragPos;

out vec4 fragColor;

uniform sampler2D skyTexture;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    vec3 viewDir = normalize(viewPos - vFragPos);
    float visibility = dot(normalize(vNormal), viewDir);

    if (visibility < 0.0) {
        discard;
    }

    // Используем текстурные координаты без модификации
    vec4 clouds = texture(skyTexture, vTexCoord);

    vec3 atmosphereColor = vec3(0.7, 0.85, 1.0);
    vec3 lightDir = normalize(lightPos - vFragPos);
    float dayFactor = max(dot(normalize(vNormal), lightDir), 0.0);

    float rim = 1.0 - visibility;
    rim = pow(rim, 3.0);

    vec3 finalColor = atmosphereColor;
    finalColor = mix(finalColor, clouds.rgb, clouds.r * 0.5 * dayFactor);
    finalColor += vec3(0.2, 0.3, 0.4) * rim;

    float alpha = clouds.r * 0.5 * dayFactor;
    alpha = mix(alpha, alpha + rim * 0.5, 0.5);
    alpha *= visibility;
    alpha = clamp(alpha, 0.0, 0.7);

    if (alpha < 0.05) {
        discard;
    }

    fragColor = vec4(finalColor, alpha);
}
