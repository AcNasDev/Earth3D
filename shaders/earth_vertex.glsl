#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

uniform mat4 mvp;
uniform mat4 model;
uniform mat3 normalMatrix;
uniform vec4 earthTextureInfo;    // xy - offset, zw - scale
uniform vec4 heightMapInfo;       // xy - offset, zw - scale
uniform vec4 normalMapInfo;       // xy - offset, zw - scale
uniform float displacementScale;

out vec2 TexCoord;
out vec2 HeightMapCoord;
out vec2 NormalMapCoord;
out vec3 WorldPos;
out vec3 WorldNormal;

void main()
{
    // Преобразуем текстурные координаты для каждого тайла
    TexCoord = texCoord * earthTextureInfo.zw + earthTextureInfo.xy;
    HeightMapCoord = texCoord * heightMapInfo.zw + heightMapInfo.xy;
    NormalMapCoord = texCoord * normalMapInfo.zw + normalMapInfo.xy;

    // Для отладки добавим проверку границ текстурных координат
    // TexCoord = clamp(TexCoord, 0.0, 1.0);

    WorldPos = vec3(model * vec4(position, 1.0));
    WorldNormal = normalMatrix * normal;
    gl_Position = mvp * vec4(position, 1.0);
}
