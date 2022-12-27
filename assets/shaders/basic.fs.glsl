#version 330 core

uniform vec3 Color;
uniform vec3 LightColor;
uniform vec3 LightDirection;

uniform float SpecularPower;
uniform float SpecularStrength;

uniform sampler2D albedo;
uniform sampler2D shadowMap;

in vec3 normal;
in vec3 position;
in vec2 texcoords;
in vec3 fragPos;
in vec4 lightSpacePosition;
in vec3 ambient;
out vec4 fColor;

/*
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
            float bias = max(0.05 * (1.0 - dot(normal, LightDirection)), 0.005); 
            if (currentDepth > closestDepth + bias) 
                shadow += 1.0f;
        }
    }

    return shadow / pow((sampleRadius * 2 + 1), 2);
}  
*/

vec2 poissonDisk[4] = vec2[](
  vec2( -0.94201624, -0.39906216 ),
  vec2( 0.94558609, -0.76890725 ),
  vec2( -0.094184101, -0.92938870 ),
  vec2( 0.34495938, 0.29387760 )
);

void main() {
    float shadow = 1.0;
    for (int i=0;i<4;i++){
        if (texture(shadowMap, lightSpacePosition.xy + poissonDisk[i] / 700.0).r < lightSpacePosition.z) {
            shadow -= 0.2;
        }
    }
    // float shadow = ShadowCalculation(lightSpacePosition);
    float diffuse = max(0.0, dot(normal, LightDirection));
    vec3 scatteredLight = ambient + LightColor * diffuse * shadow;

    vec3 viewDir = normalize(-position);
    vec3 reflectDir = reflect(-LightDirection, normal);

    float specular = pow(max(dot(viewDir, reflectDir), 0.0), SpecularPower);
    vec3 reflectedLight = SpecularStrength * specular * LightColor;

    vec3 alColor = texture(albedo, texcoords).rgb * Color;
    vec3 rgb = min(alColor * scatteredLight + reflectedLight * shadow, vec3(1.0));

    fColor = vec4(rgb, 1.0);
}
