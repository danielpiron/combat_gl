#version 330 core

uniform vec3 Color;
uniform vec3 Ambient;
uniform vec3 LightColor;
uniform vec3 LightDirection;

uniform float SpecularPower;
uniform float SpecularStrength;

in vec3 normal;
in vec3 position;
out vec4 fColor;

void main() {
    float diffuse = max(0.0, dot(normal, LightDirection));
    vec3 scatteredLight = Ambient + LightColor * diffuse;

    vec3 viewDir = normalize(-position);
    vec3 reflectDir = reflect(-LightDirection, normal);

    float specular = pow(max(dot(viewDir, reflectDir), 0.0), SpecularPower);
    vec3 reflectedLight = SpecularStrength * specular * LightColor;

    vec3 rgb = min(Color * scatteredLight + reflectedLight, vec3(1.0));

    fColor = vec4(rgb, 1.0);
}
