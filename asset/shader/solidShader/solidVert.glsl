#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normals;

#ifdef USE_SKINNING
#include "../core/skinning.glsl"
#endif

#include "../core/camera.glsl"
uniform mat3 normalMatrix;

out vec3 vNormal;

void main() {
    vec3 finalPos = pos;
    vec3 finalNormal = normals;

    #ifdef USE_SKINNING
    finalPos = GetSkinnedPosition(pos);
    finalNormal = GetSkinnedNormal(normals);
    #endif

    vNormal = normalize(normalMatrix * finalNormal);
    gl_Position = projection * view * model * vec4(finalPos, 1.0f);
}