#ifndef MAX_LIGHTS
#define MAX_LIGHTS 16
#endif
struct Light { vec4 posType; vec4 dirRange; vec4 colorInt; vec4 spot; };
layout(std140) uniform Lights { Light lights[MAX_LIGHTS]; int lightCount; };

struct LightSample { vec3 L; vec3 radiance; };

LightSample SampleLight(int i, vec3 fragPos) {
    Light l = lights[i];
    LightSample s;
    vec3 color = l.colorInt.rgb * l.colorInt.a;
    int type = int(l.posType.w + 0.5);
    vec3 dir = normalize(l.dirRange.xyz);
    if (type == 0) { s.L = -dir; s.radiance = color; return s; }

    vec3 d = l.posType.xyz - fragPos;
    float dist = length(d);
    s.L = d / max(dist, 0.0001);
    float a = clamp(1.0 - dist / l.dirRange.w, 0.0, 1.0);
    float atten = a * a;
    if (type == 2) atten *= smoothstep(l.spot.y, l.spot.x, dot(-s.L, dir));
    s.radiance = color * atten;
    return s;
}
