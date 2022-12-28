#version 330 core

#define PI 3.1415926535897932384626433832795

uniform vec3 Color;
uniform vec3 LightColor;
uniform vec3 LightDirection;

uniform float MetallicFactor;
uniform float RoughnessFactor;

uniform sampler2D albedo;
uniform sampler2D shadowMap;

in vec3 normal;
in vec3 position;
in vec2 texcoords;
in vec3 fragPos;
in vec4 lightSpacePosition;
in vec3 ambient;
out vec4 fColor;

vec2 poissonDisk[4] = vec2[](
  vec2( -0.94201624, -0.39906216 ),
  vec2( 0.94558609, -0.76890725 ),
  vec2( -0.094184101, -0.92938870 ),
  vec2( 0.34495938, 0.29387760 )
);

float ggxDistribution(float normalDotHalf, float roughness) {
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float normalDotHalf2 = normalDotHalf * normalDotHalf;

    float numerator = alpha2;
    float demoninator = (normalDotHalf2 * (alpha2 - 1.0) + 1.0f);
    demoninator = PI * demoninator * demoninator;

    return numerator / demoninator;
}

float geomSmith(float dp, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return dp / (dp * (1 - k) + k);
}

vec3 schlickFresnel(float dotProduct , vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - dotProduct , 0.0, 1.0), 5.0);
}

void main() {
    float shadow = 1.0;
    for (int i=0;i<4;i++){
        if (texture(shadowMap, lightSpacePosition.xy + poissonDisk[i] / 700.0).r < lightSpacePosition.z) {
            shadow -= 0.2;
        }
    }

    vec3 lightVector = LightDirection;
    vec3 viewVector = normalize(-position);
    vec3 halfVector = normalize(viewVector + lightVector);

    vec3 surfaceColor = Color * texture(albedo, texcoords).rgb;

    float normDotLight = max(dot(normal, lightVector), 0.0);
    float normDotHalf = max(dot(normal, halfVector), 0.0);
    float normDotView = max(dot(normal, viewVector), 0.0);
    float viewDotHalf = max(dot(viewVector, halfVector), 0.0);


    vec3 F0 = vec3(0.04);
    F0 = mix(F0, surfaceColor, MetallicFactor);
    vec3 F = schlickFresnel(viewDotHalf, F0);

    vec3 kS = F;
    vec3 kD = 1.0 - F;
     
    vec3 diffuseBRDF = kD * surfaceColor;

    vec3 specularBRDF = (kS * ggxDistribution(normDotHalf, RoughnessFactor) * geomSmith(normDotView, RoughnessFactor) *
                         geomSmith(normDotLight, RoughnessFactor)) / (4.0 * normDotView * normDotLight + 0.0001);

    fColor = vec4(ambient * surfaceColor + shadow * (diffuseBRDF + specularBRDF) * normDotLight, 1.0);
}
