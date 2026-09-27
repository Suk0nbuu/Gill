#pragma once
#include "mathpp.hpp"
#include "editor/gizmo/transformData.hpp"
class Gizmo;
class TransformController;
class SelectionManager;



class GizmoAdapter {
public:
    void Init(Gizmo* gizmo, TransformController* controller, TransformData* gizmoData, SelectionManager* selectionManager);

    void OnMouseMove(int mx, int my); // call every frame regardless of button state — drives hover highlight
    bool OnMouseDown(int mx, int my, const mathpp::mat4f& view, const mathpp::mat4f& proj);
    void OnMouseUp();

private:
    Gizmo* p_gizmo;
    TransformController* p_controller;
    TransformData* p_gizmoData;
    SelectionManager* p_selectionManager;
};