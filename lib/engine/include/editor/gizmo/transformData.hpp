#pragma once
#include "mathpp.hpp"
#include <cstdint>


enum class TransformAxis : uint32_t {
    None = 0, //only used for Shortcut based transform
    X = 1,
    Y = 2,
    Z = 3
};

inline TransformAxis& operator++(TransformAxis& t) {
    auto i = static_cast<uint32_t>(t);
    i++;
    t = static_cast<TransformAxis>(i);
    return t;
}

inline TransformAxis& operator+(TransformAxis& t, uint32_t i) {
    auto j = static_cast<uint32_t>(t);
    j = j + i ;
    t = static_cast<TransformAxis>(j);
    return t;
}

enum class TransformMode : int {
    Translate = 0,
    Rotate = 1,
    Scale = 2
};

enum class GizmoPlane : int {
    None = 0,
    XZ = 1,
    YX = 2,
    ZY = 3
};

enum class ReferenceFrame : int {
    Local = 0,
    World = 1
};

struct TransformData {
    TransformAxis axis = TransformAxis::None;
    GizmoPlane plane = GizmoPlane::None;
    ReferenceFrame referenceFrame = ReferenceFrame::World;
    TransformMode mode = TransformMode::Translate;
    bool visible = true;
};

inline void SwitchMode(TransformData* gizmoData) {

    if (gizmoData->mode == TransformMode::Translate) {gizmoData->mode = TransformMode::Rotate; }
    else if (gizmoData->mode == TransformMode::Rotate) {gizmoData->mode = TransformMode::Scale; }
    else {gizmoData->mode = TransformMode::Translate; }
}

inline void ToggleReferenceFrame(TransformData* gizmoData) {

    gizmoData->referenceFrame  = (gizmoData->referenceFrame == ReferenceFrame::Local) ? ReferenceFrame::World : ReferenceFrame::Local;
}

struct TransformSnapshot {
    mathpp::vec3f worldPos;
    mathpp::quatf worldRot;
    mathpp::vec3f worldScale;
};

