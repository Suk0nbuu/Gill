#version 330 core
in vec3 FragPos;
out vec4 FragColor;
#include "../core/camera.glsl"
uniform float cellSize;
uniform float extent;

float gridLine(vec2 coord) {
    vec2 grid = fract(coord);
    vec2 deriv = fwidth(coord);
    vec2 lineAA = min(grid, 1.0 - grid) / deriv;
    return 1.0 - clamp(min(lineAA.x, lineAA.y), 0.0, 1.0);
}

void main() {
    vec2 coord = FragPos.xz / cellSize;
    float line = gridLine(coord);

    float dist = length(FragPos.xz - cameraPos.xz);
    float fade = 1.0 - smoothstep(0.0, extent, dist);

    float alpha = line * fade;
    if (alpha < 0.001) discard;

    FragColor = vec4(vec3(1.0), alpha);
}