#pragma once
constexpr unsigned kMaxLights = 16;
struct GpuLight {
    float posType[4];    // xyz world position, w = type
    float dirRange[4];   // xyz = direction the light points (local -Z in world), w = range
    float colorInt[4];   // rgb color, a intensity
    float spot[4];       // x = cos(inner), y = cos(outer)
};
struct LightBlock {
    GpuLight lights[kMaxLights];   // 16 * 64 = 1024
    int count;
    int pad[3];
};
static_assert(sizeof(GpuLight) == 64);
static_assert(sizeof(LightBlock) == 1040);   // std140 rounds the block up to a multiple of 16