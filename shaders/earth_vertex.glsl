#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

uniform mat4 mvp;
uniform mat4 model;
uniform sampler2D heightMap;
uniform float displacementScale;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

// Функция для вычисления новой нормали на основе карты высот
vec3 calculateNewNormal(vec2 texCoords, float epsilon)
{
    float height = texture(heightMap, texCoords).r;

    // Вычисляем высоты в соседних точках
    float heightRight = texture(heightMap, texCoords + vec2(epsilon, 0.0)).r;
    float heightUp = texture(heightMap, texCoords + vec2(0.0, epsilon)).r;

    // Вычисляем разницу высот
    vec3 tangent = normalize(vec3(1.0, (heightRight - height) * displacementScale, 0.0));
    vec3 bitangent = normalize(vec3(0.0, (heightUp - height) * displacementScale, 1.0));

    // Вычисляем новую нормаль как векторное произведение
    return normalize(cross(tangent, bitangent));
}

void main()
{
    TexCoord = texCoord;

    // Получаем значение высоты
    float height = texture(heightMap, TexCoord).r;

    // Смещаем вершину вдоль нормали
    vec3 displacedPosition = position + normal * height * displacementScale;

    // Вычисляем новую нормаль
    vec3 newNormal = calculateNewNormal(TexCoord, 0.01);
    Normal = mat3(transpose(inverse(model))) * mix(normal, newNormal, 0.5);

    // Финальная позиция
    gl_Position = mvp * vec4(displacedPosition, 1.0);
    FragPos = vec3(model * vec4(displacedPosition, 1.0));
}
