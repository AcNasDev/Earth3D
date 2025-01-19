#version 330 core

in vec3 position;
in vec2 texCoord;
in vec3 normal;
in vec2 tileCoord;

out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vFragPos;
flat out vec2 vTileCoord;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

void main() {
    vTexCoord = texCoord;
    vTileCoord = tileCoord;

    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    vNormal = normalize(normalMatrix * normal);

    vec4 worldPos = modelMatrix * vec4(position, 1.0);
    vFragPos = worldPos.xyz;

    gl_Position = projectionMatrix * viewMatrix * worldPos;
}
