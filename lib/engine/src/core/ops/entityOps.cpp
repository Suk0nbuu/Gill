#include "../../../include/core/ops/entityOps.hpp"
#include <vector>
#include <functional>
#include "scene/scene.hpp"
#include "core/system/hierarchy/hierarchy.hpp"
#include "core/system/transform/transform.hpp"
#include "core/system/mesh/mesh.hpp"
#include "core/system/material/material.hpp"
#include "io/gltf/gltf.hpp"
#include "core/project/projectPath.hpp"

namespace {
    void DestroyOne(const EngineContext& ctx, Entity e) {
        Scene* s = ctx.p_scene;

        if (auto* m = s->TryGetComponent<comp::MeshComponent>(e); m && m->meshID != AssetHandle{})
            ctx.p_meshSystem->RemoveMesh(m->meshID);
        if (auto* m = s->TryGetComponent<comp::MaterialComponent>(e); m && m->materialID != AssetHandle{})
            ctx.p_materialSystem->RemoveMaterial(m->materialID);

        if (s->TryGetComponent<comp::MeshComponent>(e))      s->RemoveComponent<comp::MeshComponent>(e);
        if (s->TryGetComponent<comp::PrimitiveComponent>(e)) s->RemoveComponent<comp::PrimitiveComponent>(e);
        if (s->TryGetComponent<comp::MaterialComponent>(e))  s->RemoveComponent<comp::MaterialComponent>(e);
        if (s->TryGetComponent<comp::NameComponent>(e))      s->RemoveComponent<comp::NameComponent>(e);
        if (s->TryGetComponent<comp::LightComponent>(e))     s->RemoveComponent<comp::LightComponent>(e);
        if (s->TryGetComponent<comp::ArmatureComponent>(e))  s->RemoveComponent<comp::ArmatureComponent>(e);

        ctx.p_hierarchy->RemoveAllChildren(e);
        ctx.p_hierarchy->Unparent(e);
        ctx.p_transformSystem->RemoveTransform(e);
        s->DestroyEntity(e);          // last, so a recycled id never inherits stale components
    }
}

void DestroyEntityTree(const EngineContext& ctx, Entity root) {
    std::vector<Entity> order;    // children before parents
    std::function<void(Entity)> collect = [&](Entity e) {
        for (Entity c : ctx.p_hierarchy->GetChild(e)) collect(c);
        order.push_back(e);
    };
    collect(root);
    for (Entity e : order) DestroyOne(ctx, e);
}

bool AddPrimitiveMesh(const EngineContext& ctx, Entity e, const comp::PrimitiveComponent& prim) {
    if (ctx.p_scene->TryGetComponent<comp::MeshComponent>(e)) return false;   // already has a mesh
    comp::MeshComponent m;
    m.meshID = ctx.p_meshSystem->AddPrimitive(prim.type, prim.rings, prim.segments, prim.radius, prim.height);
    ctx.p_scene->InsertComponent(e, m);
    ctx.p_scene->InsertComponent(e, prim);
    return true;
}

void RemoveMeshFromEntity(const EngineContext& ctx, Entity e) {
    Scene* s = ctx.p_scene;
    if (auto* m = s->TryGetComponent<comp::MeshComponent>(e); m && m->meshID != AssetHandle{})
        ctx.p_meshSystem->RemoveMesh(m->meshID);          // free the GPU mesh while the handle is still there
    if (s->TryGetComponent<comp::PrimitiveComponent>(e)) s->RemoveComponent<comp::PrimitiveComponent>(e);
    if (s->TryGetComponent<comp::MeshComponent>(e))      s->RemoveComponent<comp::MeshComponent>(e);
    // later: MeshRefComponent
}
/*
bool AddMeshFromModel(EngineContext& ctx, Entity e, const std::filesystem::path& src, std::string& err) {
    const auto& root = ctx.p_projectPaths->Root();
    if (root.empty()) { err = "no project open"; return false; }

    GLTFModel model = LoadGLTF(src.string());                      // load first, so a bad file leaves nothing in the project
    if (!model.error.empty()) { err = model.error; return false; }
    if (model.meshes.size() != 1) { err = "model has " + std::to_string(model.meshes.size()) + " meshes, use Import Model"; return false; }
    if (model.meshes[0].IsSkinned()) { err = "skinned meshes need Import Model (they need the skeleton)"; return false; }

    const std::string rel = ImportAsset(src, root, "models", err); // your ImportTexture, generalized below
    if (rel.empty()) return false;

    comp::MeshComponent mc;
    mc.meshID = ctx.p_meshSystem->RegisterMesh(std::move(model.meshes[0]));
    ctx.p_scene->InsertComponent(e, mc);
    ctx.p_scene->InsertComponent(e, comp::MeshRefComponent{rel, 0});
    return true;
}
*/


Entity SpawnEmpty(EngineContext &ctx, std::string_view name) {
    Entity entity = ctx.p_scene->CreateEntity();
    ctx.p_transformSystem->AddTransform(entity);
    if (!name.empty()) {
        comp::NameComponent nameComp;
        nameComp.name = name;
        ctx.p_scene->InsertComponent(entity, nameComp);
    }
    return entity;
}


