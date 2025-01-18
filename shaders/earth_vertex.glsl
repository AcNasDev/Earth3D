#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out mat3 TBN;

uniform mat4 mvp;
uniform mat4 model;

void main()
{
    TexCoord = texCoord;

    // Вычисляем позицию в мировом пространстве
    FragPos = vec3(model * vec4(position, 1.0));

    // Вычисляем нормаль в мировом пространстве
    Normal = mat3(transpose(inverse(model))) * normal;

    // Создаем TBN матрицу для normal mapping
    vec3 T = normalize(vec3(model * vec4(cross(normal, vec3(0.0, 0.0, 1.0)), 0.0)));
    vec3 B = normalize(vec3(model * vec4(cross(normal, T), 0.0)));
    vec3 N = normalize(vec3(model * vec4(normal, 0.0)));
    TBN = mat3(T, B, N);

    gl_Position = mvp * vec4(position, 1.0);
}
