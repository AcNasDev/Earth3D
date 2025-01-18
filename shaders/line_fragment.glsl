#version 330 core

uniform vec4 color;  // Цвет линии с альфа-каналом для прозрачности
out vec4 fragColor;

void main()
{
    fragColor = color;
}
