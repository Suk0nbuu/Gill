#pragma once
#include "imgui.h"
#include "component/entity.hpp"
#include "render/mesh/primitive.hpp"
#include "editor/registry/registry.hpp"
#include "core/system/mesh/mesh.hpp"
#include "core/system/material/material.hpp"
#include "uiComponent/mesh.hpp"
#include <vector>
#include "scene/scene.hpp"




class EditorPanel {
public:
    virtual ~EditorPanel() = default;
    virtual void Draw() = 0;
    std::string m_name;
};

class Window;
class Hierarchy;
class Gizmo;
class TransformSystem;
class Renderer;
class Input;
class EditorInputMap;
class SelectionManager;
struct TransformData;







class PropertiesPanel : public EditorPanel {
public:
    PropertiesPanel(EditorContext ctx)
        : m_ctx(ctx) {}
    void Draw() override;
private:
    EditorContext m_ctx;
    static inline const ComponentTypeInfo ComponentTypes[] = {
        { "Mesh",     AddMesh,    RemoveMesh,DrawMeshInspector,HasMesh },
    };
};

class HierarchyPanel : public EditorPanel {
    public:
    HierarchyPanel(EditorContext ctx) : m_ctx(ctx)  {}
    void Draw() override;
private:
    void DrawEntityNode(Entity entity);
    EditorContext m_ctx;
};

class GizmoPanel : public EditorPanel {
public:
    GizmoPanel(EditorContext ctx) : m_ctx(ctx) {}

    void Draw() override;

private:
    EditorContext m_ctx;
};

struct PrimitiveData {
    Entity entity = UINT32_MAX;
    PrimitiveType type;
    int rings = 32;
    int segments = 16;
    float radius = 1.0f;
    float height = 1.0f;
};

class UIManager {
public:
    void Init(Window* window,Scene* scene,TransformSystem* transformSystem, Hierarchy* hierarchy,TransformData* gizmoData,Renderer* renderer,MeshSystem* meshSystem,MaterialSystem* materialSystem,EditorInputMap* editorInputMap,SelectionManager* selectionManager,Input* input);
    void RenderPanels();
    void RenderAddMenu(Scene* scene);
    void RenderPrimitiveOp(Scene* scene);
    void RenderViewportMode();
    void BeginFrame();
    void EndFrame();
    bool WantCaptureMouse();
    void Shutdown();

private:
    void DrawDockspace(Scene* scene);
    void AdjustLastOp(Scene* scene);
    EditorContext m_ctx;
    PrimitiveData m_primitiveData;
    std::vector<std::unique_ptr<EditorPanel>> v_panels;
    void AddPrimitive(Scene* scene, PrimitiveType type);
    void AddSunlight(Scene* scene);
    ImGuiIO* p_io_ptr;



};