#include "editor/ui/uiComponent/material.hpp"
#include "scene/scene.hpp"
#include "imgui.h"

bool HasMaterial(EditorContext& ctx, Entity e) {
    return ctx.p_scene->TryGetComponent<comp::MaterialComponent>(e) != nullptr;
}

void AddMaterial(Entity e,EditorContext& ctx) {
    Material mat;
    comp::MaterialComponent comp;
    mat.shaderID = ctx.p_materialSystem->GetDefaultShader();
    comp.materialID = ctx.p_materialSystem->RegisterMaterial(std::move(mat));
    ctx.p_scene->InsertComponent(e,comp);
}

void RemoveMaterial(Entity e,EditorContext& ctx) {
   const auto& matComp = ctx.p_scene->TryGetComponent<comp::MaterialComponent>(e);
    if (!matComp) return;
    ctx.p_materialSystem->RemoveMaterial(matComp->materialID);
    ctx.p_scene->RemoveComponent<comp::MaterialComponent>(e);
}

void DrawMaterialInspector(Entity e, EditorContext& ctx) {
    const auto* matComp = ctx.p_scene->TryGetComponent<comp::MaterialComponent>(e);
    if (!matComp) return;
    if (!ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) return;

    Material* mat = (matComp->materialID != AssetHandle{})
                  ? ctx.p_materialSystem->GetMaterial(matComp->materialID) : nullptr;
    if (!mat) { ImGui::TextDisabled("Material missing"); return; }

    ImGui::ColorEdit3("Albedo", &mat->albedo.x);
    ImGui::SliderFloat("Metallic", &mat->metallic, 0.0f, 1.0f);
    ImGui::SliderFloat("Roughness", &mat->roughness, 0.05f, 1.0f);
}