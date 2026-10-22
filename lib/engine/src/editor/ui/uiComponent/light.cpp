#include "editor/ui/uiComponent/light.hpp"
#include "editor/registry/registry.hpp"
#include "imgui.h"
#include "scene/scene.hpp"

bool HasLight(const EngineContext &ctx, Entity e) {
    return ctx.p_scene->TryGetComponent<comp::LightComponent>(e) != nullptr;
}

void RemoveLightComponent(const EngineContext &ctx, Entity e) {
    const auto& lComp = ctx.p_scene->TryGetComponent<comp::LightComponent>(e);
    if (lComp == nullptr) return;
    ctx.p_scene->RemoveComponent<comp::LightComponent>(e);
}

void LightAddMenu(EditorContext &ctx, Entity e) {
    if (ctx.p_scene->TryGetComponent<comp::LightComponent>(e) != nullptr) return;
    // DrawAddMenu runs every frame the popup is open, so only insert when the item is clicked
    if (!ImGui::MenuItem("Light")) return;
    comp::LightComponent lComp;
    ctx.p_scene->InsertComponent(e,lComp);
}


bool DrawLightInspector(EditorContext &ctx, Entity e) {
    const auto& l = ctx.p_scene->TryGetComponent<comp::LightComponent>(e);
    if (!l) return false;
    bool changed = false;
    int t = int(l->type);
    if (ImGui::Combo("Type", &t, "Directional\0Point\0Spot\0")) { l->type = comp::LightType(t); changed = true; }
    changed |= ImGui::ColorEdit3("Color", &l->color.x);
    changed |= ImGui::SliderFloat("Intensity", &l->intensity, 0.0f, 10.0f);
    changed |= ImGui::SliderFloat("Range", &l->range, 0.05f, 100.0f);
    changed |= ImGui::SliderAngle("SpotSize", &l->spotSize, 0.0f, 60.0f);
    changed |= ImGui::SliderFloat("Blend",&l->blend, 0.1f, 1.0f);
    return changed;


}
