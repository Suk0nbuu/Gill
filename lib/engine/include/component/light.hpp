#pragma once

enum class LightType { Directional, Point, Spot };
namespace comp {
    struct LightComponent {
        LightType type = LightType::Point;
        mathpp::vec3f color{1, 1, 1};
        float intensity = 1.0f;
        float range = 10.0f;        // point/spot: where the light fades to zero
        float innerAngle = 20.0f;   // spot only
        float outerAngle = 30.0f;   // spot only
    };
}