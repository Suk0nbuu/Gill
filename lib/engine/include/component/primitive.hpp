#pragma once
#include <cstdint>
using Entity = uint32_t;
enum class PrimitiveType{
    Cube,
    Plane,
    Sphere,
    Cylinder,
    Cone
};
struct PrimitiveData {
    Entity entity = UINT32_MAX;
    PrimitiveType type;
    int rings = 32;
    int segments = 16;
    float radius = 1.0f;
    float height = 1.0f;
};
namespace comp {
    struct PrimitiveComponent {
        PrimitiveType type = PrimitiveType::Cube;
        int rings = 32;
        int segments = 16;
        float radius = 1.0f;
        float height = 1.0f;
    };
}