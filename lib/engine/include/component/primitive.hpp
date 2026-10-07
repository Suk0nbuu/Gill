#pragma once
enum class PrimitiveType{
    Cube,
    Plane,
    Sphere,
    Cylinder,
    Cone
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