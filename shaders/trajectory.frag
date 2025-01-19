#version 330 core
in float vLineCoord;
out vec4 FragColor;

uniform vec4 color;
uniform float time;

void main() {
    float pattern = fract(vLineCoord - time); // Анимированный паттерн
    float dash = step(pattern, 0.5); // Создаем пунктир

    // Для предсказанной траектории используем постепенное затухание
    float fadeOut = color.a;
    if(color.b > 0.5) { // Проверяем, является ли линия предсказанной (голубой)
        fadeOut *= smoothstep(1.0, 0.0, vLineCoord * 0.1);
    }

    FragColor = vec4(color.rgb, color.a * dash * fadeOut);
}
