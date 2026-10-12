#version 330 core

#define PI 3.14159265359

#include "../../core/camera.glsl"
#include "../../core/light.glsl"
in vec3 normals;
in vec3 fragPos;


uniform vec3 albedo;

// Added Metallic parameter for full PBR support
uniform float metallic = 0.0; 
uniform float roughness = 0.5;

out vec4 fragColor;

vec3 ComputeHalf(vec3 light, vec3 view) {
    return normalize(light + view);
}

float NormalDistribute(float rough, vec3 normal, vec3 halfVec) {
    float a = rough * rough;
    float a2 = a * a;
    float NdotH = max(dot(normal, halfVec), 0.0);
    float NdotHSq = NdotH * NdotH;

    float denom = (NdotHSq * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return a2 / max(denom, 0.0001);
}

float GeometryFactor(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0; // Direct lighting k parameter

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return num / max(denom, 0.0001);
}

float GeometryTerm(vec3 view, vec3 light, vec3 normal, float rough) {
    float NdotV = max(dot(normal, view), 0.0);
    float NdotL = max(dot(normal, light), 0.0);
    
    return GeometryFactor(NdotV, rough) * GeometryFactor(NdotL, rough);
}

vec3 Schlick_Approx(float VdotH, vec3 F0) {
    return F0 + (vec3(1.0) - F0) * pow(clamp(1.0 - VdotH, 0.0, 1.0), 5.0);
}

vec3 BRDFShader(vec3 albedo, vec3 normal, vec3 view, vec3 light, float rough, float metal) {
    vec3 halfVec = ComputeHalf(light, view);

    float NdotL = max(dot(normal, light), 0.0);
    float NdotV = max(dot(normal, view), 0.0);
    float VdotH = max(dot(view, halfVec), 0.0);

    // Default base reflectivity: 0.04 for dielectrics, tinted albedo for metals
    vec3 F0 = mix(vec3(0.04), albedo, metal);

    // Cook-Torrance Terms
    float D = NormalDistribute(rough, normal, halfVec);
    float G = GeometryTerm(view, light, normal, rough);
    vec3  F = Schlick_Approx(VdotH, F0);

    // Specular BRDF
    vec3 numerator = D * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.0001; // Epsilon prevents div-by-zero
    vec3 specular = numerator / denominator;

    // Energy Conservation: kS is Fresnel (F), kD is remaining diffuse
    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metal); // Pure metals have zero diffuse

    // Final Radiance
    vec3 diffuse = kD * albedo / PI;
    return (diffuse + specular) * NdotL;
}

void main() {
    vec3 N = normalize(normals);
    vec3 V = normalize(cameraPos.xyz - fragPos);

    vec3 color = vec3(0.03) * albedo;                 // ambient floor, so unlit sides aren't black
    for (int i = 0; i < lightCount; ++i) {
        LightSample s = SampleLight(i, fragPos);
        color += BRDFShader(albedo, N, V, s.L, roughness, metallic) * s.radiance;
    }
    color = color / (color + vec3(1.0));              // existing tone map + gamma, unchanged
    color = pow(color, vec3(1.0 / 2.2));
    fragColor = vec4(color, 1.0);
}
