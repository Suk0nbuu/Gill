#pragma once
#include <functional>
#include <vector>
#include <cstdint>
#include "core/system/mesh/mesh.hpp"
#include "core/system/material/material.hpp"
#include "core/engine/engineContext.hpp"
class Scene;

using Entity = uint32_t;

struct TransformData;

class TransformSystem;
class Renderer;
class EditorInputMap;
class SelectionManager;
class Hierarchy;
class Input;

struct EditorContext : EngineContext {
    Input* p_input;
    TransformData* p_gizmoData;
    Renderer* p_renderer;
    EditorInputMap* p_editorInputMap;
    SelectionManager* p_selectionManager;
};



