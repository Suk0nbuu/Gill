#include "imgui.h"
#include "magic_enum.hpp"
#include "editor/registry/registry.hpp"
#include "scene/scene.hpp"
#include "render/mesh/primitive.hpp"
#include "editor/ui/uiComponent/mesh.hpp"

void AddMesh(Entity e,EditorContext& ctx) {
    ctx.p_scene->InsertComponent(e,comp::MeshComponent{});
}

void RemoveMesh(EditorContext& ctx, Entity e) {
   ctx.p_scene->RemoveComponent<comp::MeshComponent>(e);
}
bool HasMesh(EditorContext& ctx, Entity e) {
   return ctx.p_scene->TryGetComponent<comp::MeshComponent>(e) != nullptr;
}

void DrawMeshInspector(Entity e,EditorContext& ctx) {
    auto* mesh = ctx.p_scene->TryGetComponent<comp::MeshComponent>(e);
    if (mesh->meshID == AssetHandle{}) {
        constexpr auto count = magic_enum::enum_count<PrimitiveType>();
        constexpr auto names = magic_enum::enum_names<PrimitiveType>();
        constexpr auto values = magic_enum::enum_values<PrimitiveType>();

        if (ImGui::BeginListBox("Meshes"))
        {
            for (size_t  i = 0; i <count  ; i++)
            {
                if (ImGui::Selectable(names[i].data())) {
                    mesh->meshID= ctx.p_meshSystem->AddPrimitive(values[i]);
                }
            }
            ImGui::EndListBox();

        }
    }
    else {
        {ImGui::Text("Mesh ID: %u", mesh->meshID.index);}
    }
}


void MeshAddMenu(EditorContext& ctx, Entity e) {
    if (!ImGui::BeginMenu("Mesh")) return;
    if (ImGui::BeginMenu("Primitive")) {
        for (auto t : magic_enum::enum_values<PrimitiveType>())
            if (ImGui::MenuItem(std::string(magic_enum::enum_name(t)).c_str()))
                AddPrimitiveMesh(ctx, e, DefaultPrimitive(t));      // the engine op from meshOps
        ImGui::EndMenu();
    }
    // later: if (ImGui::MenuItem("From Asset...")) { ... }
    ImGui::EndMenu();
}