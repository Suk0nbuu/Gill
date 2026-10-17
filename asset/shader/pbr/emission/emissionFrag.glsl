#version 330 core

out vec4 fragColor;

uniform vec3 color = vec3(1.0);
uniform float strength = 1.0f;

void main (){
    vec3 c = color * strength;

    fragColor = vec4(c,1.0);
}