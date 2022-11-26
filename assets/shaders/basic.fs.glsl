#version 330 core

uniform vec3 Color;
uniform vec3 Ambient;
uniform vec3 LightColor;
uniform vec3 LightDirection;

in vec3 normal;
out vec4 fColor;

void main() {
    float diffuse = max(0.0, dot(normal, LightDirection));
    vec3 scatteredLight = Ambient + LightColor * diffuse;

    vec3 rgb = min(Color * scatteredLight, vec3(1.0));

    fColor = vec4(rgb, 1.0);
}
