#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 anormals;
layout (location = 2) in vec2 UV;

uniform mat4 model;

#ifdef USE_SKINNING
#include "skinning.glsl"
#endif

#include "camera.glsl"

out vec3 normals;
out vec3 fragPos;
out vec2 vUV;

void main() {
    vec3 finalPos    = position;
    vec3 finalNormal = anormals;
    #ifdef USE_SKINNING
    finalPos    = GetSkinnedPosition(position);
    finalNormal = GetSkinnedNormal(anormals);
    #endif
    vUV = UV;
    gl_Position = projection * view * model * vec4(finalPos, 1.0);
    normals = mat3(transpose(inverse(model))) * finalNormal;
    fragPos = vec3(model * vec4(finalPos, 1.0));
}