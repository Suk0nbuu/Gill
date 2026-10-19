#include "editor/ui/uiComponent/material.hpp"
#include "scene/scene.hpp"
#include "editor/registry/registry.hpp"
#include "imgui.h"
#include "editor/dialog/filePicker.hpp"
#include <filesystem>
#include <optional>
#include "stb_image.h"
#include "core/project/projectPath.hpp"

namespace {
    std::string ImportTexture(const std::filesystem::path& src, const std::filesystem::path& root, std::string& err) {
        int w, h, c;
        if (!stbi_info(src.string().c_str(), &w, &h, &c)) { err = "not a readable image"; return {}; }  // checks content, not extension
        std::error_code ec;
        const auto dir = root / "assets" / "textures";
        std::filesystem::create_directories(dir, ec);
        auto dst = dir / src.filename();
        for (int i = 1; std::filesystem::exists(dst, ec) && std::filesystem::file_size(dst, ec) != std::filesystem::file_size(src, ec); ++i)
            dst = dir / (src.stem().string() + "_" + std::to_string(i) + src.extension().string());
        if (!std::filesystem::exists(dst, ec)) {
            std::filesystem::copy_file(src, dst, ec);
            if (ec) { err = "copy failed: " + ec.message(); return {}; }
        }
        return std::filesystem::relative(dst, root, ec).generic_string();
    }
}
bool HasMaterial(const EngineContext& ctx, Entity e) {
    return ctx.p_scene->TryGetComponent<comp::MaterialComponent>(e) != nullptr;
}

void MaterialAddMenu(EditorContext& ctx, Entity e) {
    if (!ImGui::MenuItem("Material")) return;
    if (HasMaterial(ctx, e)) return;
    Material mat = ctx.p_materialSystem->MakeMaterial("builtin:brdf");
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
    const ShaderDef* def = ctx.p_materialSystem->GetShaderDef(mat->shaderKey);
    if (ImGui::BeginCombo("Shader", mat->shaderKey.c_str())) {
        for (const auto& k : ctx.p_materialSystem->ShaderKeys())
            if (ImGui::Selectable(k.c_str(), k == mat->shaderKey) && k != mat->shaderKey) {
                ctx.p_materialSystem->SetShader(*mat, k); changed = true; break;
            }
        ImGui::EndCombo();
    }
    for (auto& p : mat->props) {
        const PropertyDecl* d = def ? def->Find(p.name) : nullptr;               // Find(): linear search by name
        const float lo = d ? d->min : 0.0f, hi = d ? d->max : 1.0f;

        if (auto* f = std::get_if<float>(&p.value))
            changed |= ImGui::SliderFloat(p.name.c_str(), f, lo, hi);
        else if (auto* v = std::get_if<mathpp::vec3f>(&p.value))
            changed |= ImGui::ColorEdit3(p.name.c_str(), &v->x);
        else if (auto* t = std::get_if<TextureRef>(&p.value)) {
            ImGui::PushID(p.name.c_str());
            ImGui::TextUnformatted(p.name.c_str()); ImGui::SameLine();
            const std::string label = t->path.empty() ? "None (click to choose)" : std::filesystem::path(t->path).filename().string();
            const std::filesystem::path& root = ctx.p_projectPaths->Root();
            if (root.empty()) { ImGui::TextDisabled("No project open"); }
            if (ImGui::Button(label.c_str())) {
                if (auto picked = FilePicker::PickImageFile()) {
                    std::string err;
                    const std::string rel = ImportTexture(*picked, root, err);
                    if (!rel.empty()) { t->path = rel; t->handle = ctx.p_materialSystem->GetOrLoadTexture(rel); changed = true; }
                }
            }
            if (!t->path.empty()) { ImGui::SameLine(); if (ImGui::SmallButton("x")) { *t = {}; changed = true; } }
            ImGui::PopID();
        }
    }
    return changed;
}