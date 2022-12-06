#version 330 core
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;

uniform mat4 MVPMatrix;
uniform mat4 Model;
uniform mat4 ModelMatrix;
uniform mat4 ModelViewMatrix;
uniform mat4 LightViewMatrix;
uniform mat3 NormalMatrix;

out vec3 position;
out vec4 lightSpacePosition;
out vec3 normal;

void main() {
    normal = normalize(NormalMatrix * vNormal);
    position = (ModelViewMatrix * vec4(vPosition, 1)).xyz;
    vec3 fragPos = (Model * vec4(vPosition, 1)).xyz;
    lightSpacePosition = LightViewMatrix * vec4(fragPos, 1);
    gl_Position = MVPMatrix * vec4(vPosition, 1);
}