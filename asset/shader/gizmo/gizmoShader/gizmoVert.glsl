#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec2 UV;

uniform mat4 model;
#include "../../core/camera.glsl"



void main()
{
    gl_Position = projection * view * model * vec4(position,1.0f);
    //gl_Position = vec4(0.0,0.0,0.0,1.0);
}
