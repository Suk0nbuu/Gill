#pragma once
#include "mathpp.hpp"


struct Vertex {
    mathpp::vec3f position;
    mathpp::vec3f normal;
    mathpp::vec2f uv;
};

struct SkinnedVertex {
    mathpp::vec3f position;
    mathpp::vec3f normal;
    mathpp::vec2f uv;
    mathpp::vec4i boneIndices{-1, -1, -1, -1};
    mathpp::vec4f boneWeights{0.0f, 0.0f, 0.0f, 0.0f};
};