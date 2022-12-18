#version 330 core
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoords;

uniform mat4 MVPMatrix;
uniform mat4 ModelMatrix;
uniform mat4 ModelViewMatrix;
uniform mat4 LightViewMatrix;
uniform mat3 NormalMatrix;

uniform vec3 AmbientSky;
uniform vec3 AmbientEquator;
uniform vec3 AmbientGround;

out vec3 ambient;
out vec3 position;
out vec4 lightSpacePosition;
out vec3 normal;
out vec2 texcoords;

void main() {
    normal = normalize(NormalMatrix * vNormal);
    position = (ModelViewMatrix * vec4(vPosition, 1)).xyz;
    lightSpacePosition = LightViewMatrix * vec4(vPosition, 1);
    texcoords = vTexCoords;
    gl_Position = MVPMatrix * vec4(vPosition, 1);
    ambient = normal.y > 0 ? mix(AmbientEquator, AmbientSky, normal.y) : mix(AmbientEquator, AmbientGround, -normal.y);
}