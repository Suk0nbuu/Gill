#include "editor/ui/uiComponent/mesh.hpp"
#include <string>
#include "imgui.h"
#include "magic_enum.hpp"
#include "editor/registry/registry.hpp"
#include "scene/scene.hpp"
#include "core/ops/entityOps.hpp"
#include "editor/dialog/filePicker.hpp"

namespace {
    comp::PrimitiveComponent DefaultPrimitive(PrimitiveType t) {
        comp::PrimitiveComponent p;
        p.type = t;
        return p;
    }
}

bool HasMesh(const EngineContext& ctx, Entity e) {
    return ctx.p_scene->TryGetComponent<comp::MeshComponent>(e) != nullptr;
}

void RemoveMesh(const EngineContext& ctx, Entity e) {
    RemoveMeshFromEntity(ctx, e);          // frees the GPU mesh and both components
}

bool DrawMeshInspector(EditorContext& ctx, Entity e) {
    if (auto* prim = ctx.p_scene->TryGetComponent<comp::PrimitiveComponent>(e))
        ImGui::TextDisabled("Source: Primitive (%s)", std::string(magic_enum::enum_name(prim->type)).c_str());
    else if (auto* ref = ctx.p_scene->TryGetComponent<comp::MeshRefComponent>(e)) {
        ImGui::TextDisabled("Source: Model file");
        ImGui::TextDisabled("%s  (mesh %d)", ref->path.c_str(), ref->index);
    }
    else
        ImGui::TextDisabled("Source: none");
    return false;     // primitive fields move in here in a later step
}

void MeshAddMenu(EditorContext& ctx, Entity e) {
    if (!ImGui::BeginMenu("Mesh")) return;
    if (ImGui::BeginMenu("Primitive")) {
        for (auto t : magic_enum::enum_values<PrimitiveType>())
            if (ImGui::MenuItem(std::string(magic_enum::enum_name(t)).c_str()))
                AddPrimitiveMesh(ctx, e, DefaultPrimitive(t));
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Model file (.glb)...")) {
        if (auto picked = FilePicker::PickModelFile()) {
            std::string err;
            if (!AddMeshFromModel(ctx, e, *picked, err)) std::fprintf(stderr, "Add mesh failed: %s\n", err.c_str());
        }
    }
    ImGui::EndMenu();
}