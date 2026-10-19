#include "core/ops/lightOps.hpp"

#include "component/primitive.hpp"
#include "core/engine/engineContext.hpp"
#include "core/system/transform/transform.hpp"
#include "core/system/mesh/mesh.hpp"
#include "component/mesh.hpp"
#include "scene/scene.hpp"
Entity SpawnLight(EngineContext& ctx, comp::LightType  type) {
    Entity entity = ctx.p_scene->CreateEntity();
    ctx.p_transformSystem->AddTransform(entity);
    comp::LightComponent lComp;
    comp::PrimitiveComponent pComp;
    pComp.type = PrimitiveType::Sphere;
    comp::MeshComponent mComp;
    mComp.meshID = ctx.p_meshSystem->AddPrimitive(pComp.type,pComp.rings,pComp.segments,pComp.radius,pComp.height);
    lComp.type = type;
    ctx.p_scene->InsertComponent(entity,lComp);
    ctx.p_scene->InsertComponent(entity,pComp);
    return entity;
}