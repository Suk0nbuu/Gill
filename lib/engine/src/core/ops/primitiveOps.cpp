#include "core/ops/primitiveOps.hpp"
#include "core/engine/engineContext.hpp"
#include "core/system/mesh/mesh.hpp"
#include "core/system/transform/transform.hpp"
#include "scene/scene.hpp"

Entity SpawnPrimitive(EngineContext &ctx, PrimitiveType type,PrimitiveData& data) {
    Entity entity = ctx.p_scene->CreateEntity();
    comp::PrimitiveComponent primComp;
    primComp.type = type;
    primComp.height = data.height;
    primComp.radius = data.radius;
    primComp.rings = data.rings;
    primComp.segments = data.segments;
    ctx.p_transformSystem->AddTransform(entity);
    comp::MeshComponent meshComp;
    meshComp.meshID = ctx.p_meshSystem->AddPrimitive(primComp.type,primComp.rings,primComp.segments,primComp.radius,primComp.height);
    ctx.p_scene->InsertComponent(entity,meshComp);
    ctx.p_scene->InsertComponent(entity,primComp);
    return entity;
}

void RegeneratePrimitive(EngineContext& ctx, Entity entity) {
    const comp::PrimitiveComponent* prim = ctx.p_scene->TryGetComponent<comp::PrimitiveComponent>(entity);
    comp::MeshComponent* meshComp = ctx.p_scene->TryGetComponent<comp::MeshComponent>(entity);
    if (prim == nullptr || meshComp == nullptr) return;
    if (meshComp->meshID != AssetHandle{}) ctx.p_meshSystem->RemoveMesh(meshComp->meshID);
    meshComp->meshID = ctx.p_meshSystem->AddPrimitive(prim->type, prim->rings, prim->segments, prim->radius, prim->height);
}