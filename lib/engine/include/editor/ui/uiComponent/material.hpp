#pragma once
#include "component/entity.hpp"
#include "core/engine/engineContext.hpp"
struct EditorContext;

bool HasMaterial(const EngineContext& ctx, Entity e);
bool DrawMaterialInspector(EditorContext& ctx, Entity e);
void MaterialAddMenu(EditorContext& ctx, Entity e);
void RemoveMaterialComponent(const EngineContext& ctx, Entity e);