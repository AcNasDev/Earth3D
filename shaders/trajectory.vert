#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 mvp;
out float vLineCoord;

void main() {
    gl_Position = mvp * vec4(aPos, 1.0);
    vLineCoord = float(gl_VertexID) * 0.1; // Масштабируем координату для пунктира
}
