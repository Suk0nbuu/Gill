#pragma once
#include <memory>
#include "mathpp.hpp"
#include "core/component/event/event.hpp"
#include <utility>
#include "../include/editor/gizmo/gizmo.hpp"
#include <GLFW/glfw3.h>
#include "editorMode.hpp"
#include "editor/picker/picker.hpp"

#include "editor.hpp"
#include "GLFW/glfw3native.h"


    class Outline;
    class Renderer;
    class TransformController;
    class Input;
    class UIManager;
    class GridRenderer;
    class Window;
    class Scene;
    class Selector;
    class Camera;
    class TransformSystem;
    class Hierarchy;
    class MeshSystem;
    class MaterialSystem;
    class EditorInputMap;
    class SelectionManager;
    class ArmatureSystem;
    class GizmoAdapter;
    class ProjectPaths;
    struct ViewportInfo;
    class Framebuffer;
    using MouseEvent = EventDelegate<int,int>;





    class Editor {
    public:
        Editor();
        void Init(float width, float height,Window* window,Scene* scene,Input* input,const mathpp::mat4f& projection,Camera* camera,TransformSystem* transformSystem,Hierarchy* hierarchy,MeshSystem* meshSystem, MaterialSystem* materialSystem,Renderer* renderer,ArmatureSystem* armatureSystem,ProjectPaths* projectPaths);
        void Update(float deltaT);       // input + modal transform logic; no GL drawing
        void RenderOverlays();           // grid, gizmo, pick passes, outline; call with the viewport FBO bound
        void RunUI();                    // ImGui frame; call with the window framebuffer bound
        void BeginFrameInput();          // call once per frame, before Input::Update
        bool IsViewportActive() const { return m_viewportActive; }
        const  ViewportInfo& GetViewportInfo();
        Framebuffer& GetViewportFramebuffer();
        void SetProjection(const mathpp::mat4f& proj);
        void ShutDown();
        ~Editor();

        EventDelegate<> saveRequested;
        EventDelegate<> closeRequested;
    private:
        float m_width,m_height;
        EditorMode m_mode = EditorMode::ObjectMode;
        void TrySelect(int mx, int my);
        mathpp::vec3f ComputeMedianPos();
        void UpdateCursorForModalDrag();
        void SyncViewportSize(int w, int h);
        mathpp::vec2f LocalCursor() const;   // cursor position relative to the viewport image's top-left

        bool m_viewportActive = false;   // the 3D view owns the mouse (latched while a button is held)
        bool m_mouseWasDown = false;
        int m_vpWidth = 0, m_vpHeight = 0;
        int m_cursorMode = -1;

        std::unique_ptr<Gizmo> up_gizmo;
        std::unique_ptr<GridRenderer> up_gridRenderer;
        Input* p_input;
        std::unique_ptr<UIManager> up_ui;
        std::unique_ptr<Selector> up_selector;
        std::unique_ptr<gill_editor::Picker> up_picker;
        std::unique_ptr<SelectionManager> up_selectionManager;
        std::unique_ptr<TransformController> up_transformController;
        std::unique_ptr<GizmoAdapter> up_gizmoAdapter;
        std::unique_ptr<EditorInputMap> up_editorInputMap;
        std::unique_ptr<Outline> up_outline;
        Hierarchy* p_hierarchy;
        Camera* p_camera;
        Scene* p_scene;
        Window* p_window;
        TransformData m_gizmoData;
        MeshSystem* p_meshSystem;
        MaterialSystem* p_materialSystem;
        TransformSystem* p_transformSystem;
        ArmatureSystem* p_armatureSystem;
        Renderer* p_renderer;
        ProjectPaths* p_projectPaths;
        std::vector<std::pair<MouseEvent*, MouseEvent::Handle>> v_handles;
        mathpp::mat4f m_proj;
    };