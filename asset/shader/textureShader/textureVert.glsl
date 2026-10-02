#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 2) in vec2 UV;

#ifdef USE_SKINNING
#include "skinning.glsl"
#endif

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

out vec2 texCoord;

void main() {
    vec3 finalPos = pos;

    #ifdef USE_SKINNING
    finalPos = GetSkinnedPosition(pos);
    #endif

    texCoord = UV;
    gl_Position = projection * view * model * vec4(finalPos, 1.0f);
}