#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

uniform mat4 mvp;
uniform bool isSelected;

out vec3 fragNormal;
out vec3 fragPosition;

void main()
{
    fragPosition = position;
    fragNormal = normalize(normal);
    gl_Position = mvp * vec4(position, 1.0);
}
