#pragma once

namespace comp {
    enum class LightType : int { Directional = 0, Point = 1, Spot = 2 };
    struct LightComponent {
        LightType type = LightType::Point;
        mathpp::vec3f color{1, 1, 1};
        float intensity = 1.0f;
        float range = 10.0f;     // point/spot
        float spotSize = 45.0f;  // degrees, full cone
        float blend = 0.15f;     // 0..1, soft edge
    };
}