#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

// Матрицы трансформации
uniform mat4 mvp;
uniform mat4 model;
uniform mat3 normalMatrix;

// Текстурные сэмплеры
uniform sampler2D heightMap;  // Добавляем определение сэмплера

// Параметры смещения и текстурных координат
uniform float displacementScale;
uniform vec4 earthTextureInfo;
uniform vec4 heightMapInfo;
uniform vec4 normalMapInfo;

// Выходные переменные для фрагментного шейдера
out vec2 TexCoord;
out vec2 HeightMapCoord;
out vec2 NormalMapCoord;
out vec3 WorldPos;
out vec3 WorldNormal;

void main()
{
    // Преобразуем текстурные координаты для каждой текстуры
    TexCoord = texCoord * earthTextureInfo.zw + earthTextureInfo.xy;
    HeightMapCoord = texCoord * heightMapInfo.zw + heightMapInfo.xy;
    NormalMapCoord = texCoord * normalMapInfo.zw + normalMapInfo.xy;

    // Получаем высоту из карты высот
    float height = texture(heightMap, HeightMapCoord).r;

    // Применяем смещение по нормали
    vec3 displacedPosition = position + normal * height * displacementScale;

    // Вычисляем мировую позицию
    WorldPos = vec3(model * vec4(displacedPosition, 1.0));

    // Вычисляем нормаль в мировом пространстве
    WorldNormal = normalMatrix * normal;

    // Модифицируем нормаль на основе карты высот
    if(height > 0.0) {
        vec2 texelSize = 1.0 / textureSize(heightMap, 0);
        float heightRight = texture(heightMap, HeightMapCoord + vec2(texelSize.x, 0.0)).r;
        float heightUp = texture(heightMap, HeightMapCoord + vec2(0.0, texelSize.y)).r;

        vec3 tangent = normalize(vec3(1.0, (heightRight - height) * displacementScale * 3.0, 0.0));
        vec3 bitangent = normalize(vec3(0.0, (heightUp - height) * displacementScale * 3.0, 1.0));
        vec3 modifiedNormal = normalize(cross(tangent, bitangent));

        WorldNormal = normalMatrix * mix(normal, modifiedNormal, height * 2.0);
    }

    // Вычисляем позицию в пространстве отсечения
    gl_Position = mvp * vec4(displacedPosition, 1.0);
}
