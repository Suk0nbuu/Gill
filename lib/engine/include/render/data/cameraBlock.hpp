#pragma once
#include "mathpp.hpp"

struct CameraBlock {
    mathpp::mat4f view;         // 64
    mathpp::mat4f projection;   // 64
    float cameraPos[4];         // 16 (xyz used, w padding)
};
static_assert(sizeof(mathpp::mat4f) == 64, "std140 mat4 layout");
static_assert(sizeof(CameraBlock) == 144, "CameraBlock must match the shader block");