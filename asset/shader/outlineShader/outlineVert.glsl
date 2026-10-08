#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 model;
#include "../core/camera.glsl"
uniform float scaleFactor = 0.015f;


out vec3 normals;

void main(){
    normals = normalize(aNormal);
    vec3 newPos = (normals*scaleFactor) + aPos;
    gl_Position = projection * view * model * vec4(newPos,1.0);

}