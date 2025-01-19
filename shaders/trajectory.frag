#version 330 core
in float vLineCoord;
out vec4 FragColor;

uniform vec4 color;
uniform float time;

void main() {
    float pattern;
    float dash = 1.0;

    // Анимированный пунктир только для белой линии (орбиты)
    if(color.r == 1.0 && color.g == 1.0 && color.b == 1.0) {
        pattern = fract(vLineCoord - time); // Анимированный паттерн для белой линии
        dash = step(pattern, 0.5);
    }

    // Затухание только для голубой линии (предсказанной траектории)
    float fadeOut = 1.0;
    if(color.b == 1.0 && color.r == 0.0) {
        fadeOut = smoothstep(1.0, 0.0, vLineCoord * 0.05);
    }

    FragColor = vec4(color.rgb, color.a * dash * fadeOut);
}
