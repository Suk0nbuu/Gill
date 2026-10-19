#pragma once
#include "component/entity.hpp"
#include "component/primitive.hpp"
struct EngineContext;
Entity SpawnPrimitive(EngineContext& ctx, PrimitiveType type,PrimitiveData& data);

void RegeneratePrimitive(EngineContext& ctx, Entity entity);
