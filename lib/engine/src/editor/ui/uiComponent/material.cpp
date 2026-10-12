#include "editor/ui/uiComponent/material.hpp"
#include "scene/scene.hpp"
#include "editor/registry/registry.hpp"
#include "imgui.h"

// uiComponent/material.cpp
bool HasMaterial(const EngineContext& ctx, Entity e) {
    return ctx.p_scene->TryGetComponent<comp::MaterialComponent>(e) != nullptr;
}

void MaterialAddMenu(EditorContext& ctx, Entity e) {
    if (!ImGui::MenuItem("Material")) return;
    if (HasMaterial(ctx, e)) return;
    Material mat;
    mat.shaderID = ctx.p_materialSystem->GetDefaultShader();
    comp::MaterialComponent comp;
    comp.materialID = ctx.p_materialSystem->RegisterMaterial(std::move(mat));
    ctx.p_scene->InsertComponent(e, comp);
}

void RemoveMaterialComponent(const EngineContext& ctx, Entity e) {
    const auto* c = ctx.p_scene->TryGetComponent<comp::MaterialComponent>(e);
    if (!c) return;
    const AssetHandle id = c->materialID;
    ctx.p_scene->RemoveComponent<comp::MaterialComponent>(e);
    ctx.p_materialSystem->RemoveMaterial(id);
}

bool DrawMaterialInspector(EditorContext& ctx, Entity e) {
    const auto* c = ctx.p_scene->TryGetComponent<comp::MaterialComponent>(e);
    if (!c) return false;
    Material* mat = (c->materialID != AssetHandle{}) ? ctx.p_materialSystem->GetMaterial(c->materialID) : nullptr;
    if (!mat) { ImGui::TextDisabled("Material missing"); return false; }
    bool changed = false;
    changed |= ImGui::ColorEdit3("Albedo", &mat->albedo.x);
    changed |= ImGui::SliderFloat("Metallic", &mat->metallic, 0.0f, 1.0f);
    changed |= ImGui::SliderFloat("Roughness", &mat->roughness, 0.05f, 1.0f);
    return changed;
}