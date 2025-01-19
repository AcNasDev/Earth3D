#version 330 core
in vec2 TexCoord;
in vec3 WorldPos;
in vec3 WorldNormal;

uniform sampler2D earthTexture;

out vec4 FragColor;

void main()
{
    vec4 texColor = texture(earthTexture, TexCoord);

    // Добавим визуализацию границ тайлов
    float borderWidth = 0.01;
    vec2 tileCoord = fract(TexCoord);
    if(tileCoord.x < borderWidth || tileCoord.x > (1.0 - borderWidth) ||
       tileCoord.y < borderWidth || tileCoord.y > (1.0 - borderWidth)) {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Красные границы тайлов
    } else {
        FragColor = texColor;
    }
}
