#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 anormals;
layout (location = 2) in vec2 UV;

uniform mat4 model;

#include "../../core/camera.glsl"

out vec3 normals;
out vec4 vertexColor;

void main()
{
    gl_Position = projection * view * model * vec4(position,1.0f);
    normals = anormals;
    vertexColor = vec4(0.5f, 0.0f, 0.0f, 1.0f);
}
