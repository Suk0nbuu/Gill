#pragma once

#include "editor/registry/registry.hpp"

class Scene;

bool HasMaterial(EditorContext&, Entity e);
void AddMaterial(Entity e, EditorContext& ctx);
void RemoveMaterial(Entity e);
void DrawMaterialInspector(Entity e,EditorContext& ctx);