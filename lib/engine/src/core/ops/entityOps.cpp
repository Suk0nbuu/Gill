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

bool AddMeshFromModel(EngineContext& ctx, Entity e, const std::filesystem::path& src, std::string& err) {
    const auto& root = ctx.p_projectPaths->Root();
    if (root.empty()) { err = "no project open"; return false; }

    GLTFModel model = LoadGLTF(src.string());                      // load first, so a bad file leaves nothing in the project
    if (!model.error.empty()) { err = model.error; return false; }
    if (model.meshes.size() != 1) { err = "model has " + std::to_string(model.meshes.size()) + " meshes, use Import Model"; return false; }
    if (model.meshes[0].IsSkinned()) { err = "skinned meshes need Import Model (they need the skeleton)"; return false; }

    const std::string rel = ctx.p_projectPaths->ImportFile(src, "models", err);
    if (rel.empty()) return false;

    comp::MeshComponent mc;
    mc.meshID = ctx.p_meshSystem->RegisterMesh(std::move(model.meshes[0]));
    ctx.p_scene->InsertComponent(e, mc);
    ctx.p_scene->InsertComponent(e, comp::MeshRefComponent{rel, 0});
    return true;
}



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


void AddDefaultMaterial(EngineContext &ctx, Entity e) {
    comp::MaterialComponent mc;
    mc.materialID = ctx.p_materialSystem->RegisterMaterial(ctx.p_materialSystem->MakeMaterial("builtin:brdf"));
    ctx.p_scene->InsertComponent(e, mc);
}

std::optional<Entity> InstantiateModel(EngineContext& ctx, const std::filesystem::path& src, std::string& err) {
    GLTFModel model = LoadGLTF(src.string());
    if (!model.error.empty()) { err = model.error; return std::nullopt; }
    if (!ctx.p_projectPaths->HasRoot()) { err = "no project open"; return std::nullopt; }
    const std::string rel = ctx.p_projectPaths->ImportFile(src, "models", err);
    if (rel.empty()) return std::nullopt;

    const auto& nodes = model.nodes;
    const int count = static_cast<int>(nodes.size());
    const Entity root = SpawnEmpty(ctx, src.stem().string());

    auto isSkinnedMeshNode = [&](int i) { return nodes[i].skin >= 0 && !nodes[i].meshes.empty(); };
    auto keptAncestor = [&](int i) {                         // nearest non-joint ancestor, -1 = none
        int p = nodes[i].parent;
        while (p >= 0 && nodes[p].isJoint) p = nodes[p].parent;
        return p;
    };

    // one entity per non-joint node (joints belong to the Armature, not the ECS)
    std::vector<std::optional<Entity>> ent(count);
    for (int i = 0; i < count; ++i) {
        if (nodes[i].isJoint) continue;
        const Entity e = SpawnEmpty(ctx, nodes[i].name.empty() ? "Node" : nodes[i].name);
        ent[i] = e;
        if (!isSkinnedMeshNode(i)) {                         // glTF ignores the transform of a skinned mesh node
            ctx.p_transformSystem->SetPosition(e, nodes[i].pos);
            ctx.p_transformSystem->SetRotation(e, nodes[i].rot);
            ctx.p_transformSystem->SetScale(e, nodes[i].scale);
        }
    }

    // armature owner: the node above the skeleton's root joint (Blender's "Armature" object), else the model root
    Entity armatureOwner = root;
    if (model.armature) {
        for (int j = 0; j < count; ++j) {
            if (!nodes[j].isJoint) continue;
            const int p = nodes[j].parent;
            if (p >= 0 && nodes[p].isJoint) continue;        // not a root joint
            if (p >= 0 && ent[p]) armatureOwner = *ent[p];
            break;                                           // first root joint decides
        }
    }

    auto attach = [&](Entity child, Entity parent) {
        if (child == parent) parent = root;
        ctx.p_hierarchy->SetParent(child, parent);
        ctx.p_transformSystem->MarkDirty(child);
    };
    for (int i = 0; i < count; ++i) {
        if (!ent[i]) continue;
        Entity parent = root;
        if (isSkinnedMeshNode(i)) parent = armatureOwner;
        else if (const int a = keptAncestor(i); a >= 0 && ent[a]) parent = *ent[a];
        attach(*ent[i], parent);
    }

    // meshes; a glTF mesh used by two nodes needs its own GPU copy
    std::vector<bool> used(model.meshes.size(), false);
    auto takeMesh = [&](int idx) -> Mesh {
        if (!used[idx]) { used[idx] = true; return std::move(model.meshes[idx]); }
        GLTFModel again = LoadGLTF(src.string());
        return std::move(again.meshes[idx]);
    };
    auto addMesh = [&](Entity e, int idx) {
        comp::MeshComponent mc;
        mc.meshID = ctx.p_meshSystem->RegisterMesh(takeMesh(idx));
        ctx.p_scene->InsertComponent(e, mc);
        ctx.p_scene->InsertComponent(e, comp::MeshRefComponent{rel, idx});
        AddDefaultMaterial(ctx, e);
    };
    for (int i = 0; i < count; ++i) {
        if (!ent[i] || nodes[i].meshes.empty()) continue;
        if (nodes[i].meshes.size() == 1) { addMesh(*ent[i], nodes[i].meshes[0]); continue; }
        for (size_t k = 0; k < nodes[i].meshes.size(); ++k) {      // several primitives: one child entity each
            const Entity child = SpawnEmpty(ctx, nodes[i].name + "_" + std::to_string(k));
            attach(child, *ent[i]);
            addMesh(child, nodes[i].meshes[k]);
        }
    }

    if (model.armature) {
        comp::ArmatureComponent ac;
        ac.armature = *model.armature;
        ctx.p_armatureSystem->ResetToBindPose(ac);
        ctx.p_scene->InsertComponent(armatureOwner, ac);
    }
    return root;
}