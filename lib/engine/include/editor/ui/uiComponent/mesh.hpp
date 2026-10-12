#pragma once
#include "component/entity.hpp"
#include "core/engine/engineContext.hpp"
struct EditorContext;

bool HasMesh(const EngineContext& ctx, Entity e);
bool DrawMeshInspector(EditorContext& ctx, Entity e);
void MeshAddMenu(EditorContext& ctx, Entity e);
void RemoveMesh(const EngineContext& ctx, Entity e);