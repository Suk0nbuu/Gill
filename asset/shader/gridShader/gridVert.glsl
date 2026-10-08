#version 330 core
layout (location = 0) in vec3 aPos;

#include "../core/camera.glsl"

out vec3 FragPos;

uniform mat4 model;


void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    gl_Position = projection * view * worldPos;
}