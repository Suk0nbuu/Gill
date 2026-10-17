#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 anormals;
layout (location = 2) in vec2 UV;

uniform mat4 model;

#include "../../core/camera.glsl"

out vec3 normals;
out vec3 fragPos;
out vec2 vUV;
void main() {
    vUV = UV;
    gl_Position = projection * view * model * vec4(position, 1.0f);
    normals = mat3(transpose(inverse(model))) * anormals;   // same as celVert; optimize later
    fragPos = vec3(model * vec4(position, 1.0f));
}