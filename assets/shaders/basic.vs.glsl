#version 330 core
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;

uniform mat4 MVPMatrix;
uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;

out vec3 position;
out vec3 normal;

void main() {
    normal = normalize(NormalMatrix * vNormal);
    position = (ModelViewMatrix * vec4(vPosition, 1)).xyz;
    gl_Position = MVPMatrix * vec4(vPosition, 1);
}