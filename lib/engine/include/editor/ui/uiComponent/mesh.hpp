#pragma once
#include "editor/registry/registry.hpp"

class scene;

bool HasMesh(EditorContext& ctx, Entity e);
void AddMesh(Entity e, EditorContext& ctx);
void RemoveMesh(EditorContext&, Entity e);
void DrawMeshInspector(Entity e, EditorContext& ctx);