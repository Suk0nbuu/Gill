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
    Material* mat = ctx.p_materialSystem->GetMaterial(c->materialID);
    if (!mat) { ImGui::TextDisabled("Material missing"); return false; }

    bool changed = false;
    const ShaderDef* def = ctx.p_materialSystem->GetShaderDef(mat->shaderKey);   // add this getter, nullptr if unknown

    for (auto& p : mat->props) {
        const PropertyDecl* d = def ? def->Find(p.name) : nullptr;               // Find(): linear search by name
        const float lo = d ? d->min : 0.0f, hi = d ? d->max : 1.0f;

        if (auto* f = std::get_if<float>(&p.value))
            changed |= ImGui::SliderFloat(p.name.c_str(), f, lo, hi);
        else if (auto* v = std::get_if<mathpp::vec3f>(&p.value))
            changed |= ImGui::ColorEdit3(p.name.c_str(), &v->x);
    }
    return changed;
}