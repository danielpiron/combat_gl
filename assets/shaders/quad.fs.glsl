#version 330 core

uniform sampler2D texture1;

in vec2 TexCoords;
out vec4 fColor;

void main() {
    float depth = texture(texture1, TexCoords).r;
    fColor = vec4(depth, depth, depth, 1.0);
}
