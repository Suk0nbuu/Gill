#pragma once
constexpr unsigned kCameraBinding = 0;
constexpr unsigned kSkinBinding   = 1;
constexpr unsigned kLightBinding  = 2;
constexpr unsigned kMaxBones      = 64;

static_assert(sizeof(mathpp::mat4f) == 64);