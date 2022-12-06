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
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth - 0.005 > closestDepth  ? 1.0 : 0.0;

    return shadow;
}  

void main() {
    float diffuse = max(0.0, dot(normal, LightDirection));
    vec3 scatteredLight = Ambient + LightColor * diffuse;

    vec3 viewDir = normalize(-position);
    vec3 reflectDir = reflect(-LightDirection, normal);

    float specular = pow(max(dot(viewDir, reflectDir), 0.0), SpecularPower);
    vec3 reflectedLight = SpecularStrength * specular * LightColor;

    float shadow = ShadowCalculation(lightSpacePosition); 
    vec3 rgb = min(Color * (scatteredLight * (1.0 - shadow)) + reflectedLight, vec3(1.0));

    fColor = vec4(rgb, 1.0);
}
