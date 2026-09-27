#include "editor/gizmo/gizmoAdapter.hpp"
#include "editor/gizmo/transformController.hpp"
#include "editor/gizmo/transformData.hpp"
#include "editor/selector/selectionManager.hpp"
#include "editor/gizmo/gizmo.hpp"

void GizmoAdapter::Init(Gizmo* gizmo, TransformController* controller, TransformData* gizmoData, SelectionManager* selectionManager) {
    p_gizmo = gizmo;
    p_controller = controller;
    p_gizmoData = gizmoData;
    p_selectionManager = selectionManager;
}

void GizmoAdapter::OnMouseMove(int mx, int my) {
    if (!p_gizmoData->visible) return;
    if (!p_selectionManager->GetActiveSelected().has_value()) return;
    p_gizmo->UpdateHighlight(mx, my, p_controller->GetActiveAxis(), p_controller->IsDragging());
}

bool GizmoAdapter::OnMouseDown(int mx, int my, const mathpp::mat4f& view, const mathpp::mat4f& proj) {
    if (!p_gizmoData->visible) return false;
    if (p_controller->IsDragging()) return false;
    if (!p_selectionManager->GetActiveSelected().has_value()) return false;

    TransformAxis pickedAxis = p_gizmo->ReadAxisAt(mx, my);
    if (pickedAxis == TransformAxis::None) return false;

    p_gizmoData->axis = pickedAxis;
    p_controller->Begin(view, proj, static_cast<float>(mx), static_cast<float>(my));
    return true;
}

void GizmoAdapter::OnMouseUp() {
    if (p_controller->IsDragging()) {
        p_controller->End();
    }
}