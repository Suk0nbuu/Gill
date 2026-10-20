#version 330 core

#include "../../core/camera.glsl"
#include "../../core/light.glsl"

in vec3 normals;
in vec3 fragPos;
in vec2 vUV;

uniform sampler2D albedoTex;
uniform int has_albedoTex = 0;

uniform vec3  albedo          = vec3(1.0);
uniform vec3  shadowColor     = vec3(0.55, 0.55, 0.7);
uniform float shadowThreshold = 0.5;
uniform float shadowSoftness  = 0.02;

out vec4 fragColor;

void main() {
    vec3 N = normalize(normals);

    vec3 baseColor = albedo;
    if (has_albedoTex != 0)
    baseColor *= pow(texture(albedoTex, vUV).rgb, vec3(2.2));    // sRGB -> linear, same as brdfFrag

    float soft = max(shadowSoftness, 1e-4);                          // smoothstep needs edge0 != edge1

    vec3 lighting = vec3(0.0);
    for (int i = 0; i < lightCount; ++i) {
        LightSample s = SampleLight(i, fragPos);
        float ndl = dot(N, s.L) * 0.5 + 0.5;                         // half-lambert, 0..1
        float lit = smoothstep(shadowThreshold - soft, shadowThreshold + soft, ndl);
        lighting += mix(shadowColor, vec3(1.0), lit) * s.radiance;   // lit = full color, shadow = tinted
    }
    lighting = min(lighting, vec3(1.0));                             // fully lit = exactly the base color

    vec3 color = baseColor * lighting;
    color = pow(color, vec3(1.0 / 2.2));                             // linear -> sRGB, no tone map
    fragColor = vec4(color, 1.0);
}