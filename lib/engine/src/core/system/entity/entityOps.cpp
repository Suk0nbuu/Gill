#include "core/system/entity/entityOps.hpp"
#include <vector>
#include <functional>
#include "scene/scene.hpp"
#include "core/system/hierarchy/hierarchy.hpp"
#include "core/system/transform/transform.hpp"
#include "core/system/mesh/mesh.hpp"
#include "core/system/material/material.hpp"

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