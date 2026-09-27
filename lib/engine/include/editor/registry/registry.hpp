#pragma once
#include <functional>
#include <vector>
#include <cstdint>
#include "core/system/mesh/mesh.hpp"
#include "core/system/material/material.hpp"
class Scene;

using Entity = uint32_t;

struct TransformData;

class TransformSystem;
class Renderer;
class EditorInputMap;
class SelectionManager;
class Hierarchy;
class Input;

struct EditorContext {
    Input* p_input;
    TransformData* p_gizmoData;
    TransformSystem* p_transformSystem;
    Renderer* p_renderer;
    MeshSystem* p_meshSystem;
    MaterialSystem* p_materialSystem;
    EditorInputMap* p_editorInputMap;
    SelectionManager* p_selectionManager;
    Scene* p_scene;
    Hierarchy* p_hierarchy;
};

struct ComponentTypeInfo {
    const char* name;
    void (*Add)(Scene* s, Entity e,EditorContext&);
    void (*Remove)(Scene* s, Entity e);
    void (*DrawInspector)(Scene* s, Entity e,EditorContext&);
    bool (*Has)(Scene* s, Entity e);
};

const std::vector<ComponentTypeInfo>& GetComponentRegistry();