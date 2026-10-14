#pragma once
#include "component/entity.hpp"
#include "core/engine/engineContext.hpp"
struct EditorContext;

bool HasLight(const EngineContext& ctx, Entity e);
bool DrawLightInspector(EditorContext& ctx, Entity e);
void LightAddMenu(EditorContext& ctx, Entity e);
void RemoveLightComponent(const EngineContext& ctx, Entity e);