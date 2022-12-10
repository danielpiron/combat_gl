#version 330 core

uniform vec3 Color;
uniform vec3 Ambient;
uniform vec3 LightColor;
uniform vec3 LightDirection;

uniform float SpecularPower;
uniform float SpecularStrength;

uniform sampler2D shadowMap;

in vec3 normal;
in vec3 position;
in vec3 fragPos;
in vec4 lightSpacePosition;
out vec4 fColor;

// Pulled right out of LearnOpenGL.com
float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    float currentDepth = projCoords.z;

    float shadow = 0.0f;
    int sampleRadius = 1;
    vec2 pixelSize = 1.0 / textureSize(shadowMap, 0);
    for (int y = -sampleRadius; y <= sampleRadius; y++) {
        for (int x = -sampleRadius; x <= sampleRadius; x++) {
            float closestDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * pixelSize).r; 
            if (currentDepth > closestDepth + 0.005) 
                shadow += 1.0f;
        }
    }

    return shadow / pow((sampleRadius * 2 + 1), 2);
}  

void main() {
    float shadow = ShadowCalculation(lightSpacePosition); 
    float diffuse = max(0.0, dot(normal, LightDirection));
    vec3 scatteredLight = Ambient + LightColor * diffuse * (1.0 - shadow);

    vec3 viewDir = normalize(-position);
    vec3 reflectDir = reflect(-LightDirection, normal);

    float specular = pow(max(dot(viewDir, reflectDir), 0.0), SpecularPower);
    vec3 reflectedLight = SpecularStrength * specular * LightColor;

    vec3 rgb = min(Color * scatteredLight + reflectedLight * (1.0 - shadow), vec3(1.0));

    fColor = vec4(rgb, 1.0);
}
