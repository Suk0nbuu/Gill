
#pragma once
#include "component/entity.hpp"
#include "component/primitive.hpp"
#include "core/engine/engineContext.hpp"


void DestroyEntityTree(const EngineContext& ctx, Entity root);
bool AddPrimitiveMesh(const EngineContext& ctx, Entity e, const comp::PrimitiveComponent& prim);
void RemoveMeshFromEntity(const EngineContext& ctx, Entity e);