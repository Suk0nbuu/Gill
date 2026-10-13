#pragma once
#include "imgui.h"
#include "component/entity.hpp"
#include "render/mesh/primitive.hpp"
#include "editor/registry/registry.hpp"
#include "editor/registry/componentDescriptor.hpp"
#include "core/system/mesh/mesh.hpp"
#include "core/system/material/material.hpp"
#include "uiComponent/mesh.hpp"
#include "uiComponent/material.hpp"
#include <vector>
#include "scene/scene.hpp"
#include "core/component/event/event.hpp"

namespace {
    void RegisterBuiltinComponents(ComponentRegistry& r) {
        r.Register({ "Mesh",     HasMesh,     DrawMeshInspector,     MeshAddMenu,     RemoveMesh });
        r.Register({ "Material", HasMaterial, DrawMaterialInspector, MaterialAddMenu, RemoveMaterialComponent, "Mesh" });
    }
}




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

};

class HierarchyPanel : public EditorPanel {
    public:
    HierarchyPanel(EditorContext ctx) : m_ctx(ctx)  {}
    void Draw() override;
private:
    void DrawEntityNode(Entity entity);
    void CommitRename(Entity entity);
    void ApplyDeletion(const std::vector<Entity>& entities);
    EditorContext m_ctx;
    std::optional<Entity> m_renaming;
    std::vector<Entity> m_pendingDelete;

    char m_renameBuf[128] = {};
    bool m_focusRename = false;
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
    void RenderAddMenu();
    void RenderPrimitiveOp();
    void RenderViewportMode();
    void BeginFrame();
    void EndFrame();
    bool WantCaptureMouse();
    void Shutdown();
    void SetFileEvents(EventDelegate<>* save, EventDelegate<>* close);

private:
    void DrawDockspace(Scene* scene);
    void RegeneratePrimitiveMesh(Scene* scene,Entity entity);
    void ApplyEditorTheme();
    EditorContext m_ctx;
    PrimitiveData m_primitiveData;
    EventDelegate<>* p_saveRequested = nullptr;
    EventDelegate<>* p_closeRequested = nullptr;

    std::vector<std::unique_ptr<EditorPanel>> v_panels;
    ComponentRegistry m_registry;
    void AddPrimitive(PrimitiveType type);
    void AddEmpty(std::optional<Entity> parent = std::nullopt);
    void AddSunlight(Scene* scene);
    ImGuiIO* p_io_ptr;



};