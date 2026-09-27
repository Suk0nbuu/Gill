#pragma once
#include "mathpp.hpp"


enum class TransformAxis : int {
    None = 0,
    X = 1,
    Y = 2,
    Z = 3
};

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

