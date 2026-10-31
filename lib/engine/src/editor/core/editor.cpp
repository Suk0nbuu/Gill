#include "editor/core/editor.hpp"
#include "editor/selector/selector.hpp"
#include "render/shader/shader.hpp"
#include "editor/grid/grid.hpp"
#include "core/input/input.hpp"
#include "editor/ui/ui.hpp"
#include "core/component/camera/camera.hpp"
#include "core/system/asset/asset.hpp"
#include "core/system/mesh/mesh.hpp"
#include "core/system/material/material.hpp"
#include "core/system/hierarchy/hierarchy.hpp"
#include "editor/gizmo/transformController.hpp"
#include "core/system/transform/transform.hpp"
#include "scene/scene.hpp"
#include "core/debug/error.hpp"
#include <iostream>
#include "editor/inputAction/inputAction.hpp"
#include "editor/selector/selectionManager.hpp"
#include "editor/outline/outline.hpp"
#include "editor/gizmo/gizmoAdapter.hpp"
#include "core/debug/test.hpp"
#include "editor/ui/viewportInfo.hpp"




void Editor::Init(float Width, float Height,Window* window,Scene* scene,Input* input,const mathpp::mat4f& projection, Camera* camera,TransformSystem* transformSystem,Hierarchy* hierarchy,MeshSystem* meshSystem, MaterialSystem* materialSystem,Renderer* renderer, ArmatureSystem* armatureSystem,ProjectPaths* projectPath) {
    p_window = window;
    p_scene = scene;
    m_width = Width;
    m_height = Height;
    m_proj = projection;
    p_camera = camera;
    p_renderer = renderer;
    up_selector = std::make_unique<Selector>();
    up_gridRenderer = std::make_unique<GridRenderer>();
    up_transformController = std::make_unique<TransformController>();
    up_picker = std::make_unique<gill_editor::Picker>();
    p_hierarchy = hierarchy;
    p_transformSystem = transformSystem;
    p_meshSystem = meshSystem;
    p_materialSystem = materialSystem;
    p_input = input;
    p_projectPaths = projectPath;
    p_armatureSystem = armatureSystem;
    up_picker->Init(m_width,m_height);
    up_selectionManager = std::make_unique<SelectionManager>();
    up_editorInputMap = std::make_unique<EditorInputMap>(p_input);
    up_outline = std::make_unique<Outline>();
    up_ui = std::make_unique<UIManager>();
    up_gizmo = std::make_unique<Gizmo>();
    up_gizmoAdapter = std::make_unique<GizmoAdapter>();
    up_gridRenderer->Init(100);
    up_gizmo->Init(m_width,m_height,&m_gizmoData,p_transformSystem,up_selectionManager.get());
    up_ui->Init(window,p_scene,p_transformSystem,p_hierarchy,&m_gizmoData,p_renderer,p_meshSystem,p_materialSystem,up_editorInputMap.get(),up_selectionManager.get(),p_input,p_projectPaths);
    up_ui->SetFileEvents(&saveRequested, &closeRequested);
    up_selector->Init();
    up_outline->Init(meshSystem,transformSystem,up_selectionManager.get());
    up_transformController->Init(m_width,m_height,&m_gizmoData,p_transformSystem,up_selectionManager.get(),hierarchy);
    up_gizmoAdapter->Init(up_gizmo.get(), up_transformController.get(), &m_gizmoData, up_selectionManager.get());

    auto handle1 = p_input->mouseDown.Subscribe([this](int mx, int my) {
        if (!m_viewportActive) return;                      // the click belongs to another panel
        const ViewportInfo& vp = up_ui->GetViewportInfo();
        const int lx = mx - static_cast<int>(vp.x);         // viewport-local pixels
        const int ly = my - static_cast<int>(vp.y);
        bool consumed = up_gizmoAdapter->OnMouseDown(lx, ly, p_camera->GetViewMatrix(), m_proj);
        if (!consumed) {
            TrySelect(lx, ly);
        }
    });
    auto handle2 = p_input->mouseUp.Subscribe([this](int mx, int my) {
        up_gizmoAdapter->OnMouseUp();
    });

    v_handles.push_back(std::make_pair(&p_input->mouseDown,handle1));
    v_handles.push_back(std::make_pair(&p_input->mouseUp,handle2));
}

void Editor::BeginFrameInput() {
    // Who owns the mouse: the 3D view or the UI. While a button stays down, the answer
    // from when the press began is kept, so a drag that leaves the image doesn't hand
    // the mouse over to the UI halfway through.
    const bool anyDown = p_input->IsMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT) ||
                         p_input->IsMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT) ||
                         p_input->IsMouseButtonDown(GLFW_MOUSE_BUTTON_MIDDLE);
    if (!(anyDown && m_mouseWasDown)) m_viewportActive = up_ui->GetViewportInfo().hovered;
    m_mouseWasDown = anyDown;
}

void Editor::Update(float deltaT) {
    const ViewportInfo& vp = up_ui->GetViewportInfo();
    p_camera->Update(p_input, deltaT, {0.0f, 0.0f, 0.0f}, !m_viewportActive);
    if (vp.visible) SyncViewportSize(vp.width, vp.height);

    const mathpp::vec2f pos = LocalCursor();
    UpdateCursorForModalDrag();

    // Starting a transform needs the 3D view under the mouse and no text field focused.
    // Once a drag is running none of this is re-checked: the cursor is disabled during
    // the drag, which makes ImGui report "not hovered".
    const bool canStart = m_viewportActive && vp.visible && !ImGui::GetIO().WantTextInput;

    if (!up_transformController->IsDragging()) {
        if (!canStart) return;
        if (up_editorInputMap->IsActionPressed(EditorAction::Translate)) up_transformController->EnterMode(TransformMode::Translate, p_camera->GetViewMatrix(), m_proj, pos.x, pos.y);
        else if (up_editorInputMap->IsActionPressed(EditorAction::Rotate)) up_transformController->EnterMode(TransformMode::Rotate, p_camera->GetViewMatrix(), m_proj, pos.x, pos.y);
        else if (up_editorInputMap->IsActionPressed(EditorAction::Scale)) up_transformController->EnterMode(TransformMode::Scale, p_camera->GetViewMatrix(), m_proj, pos.x, pos.y);
    } else {
        if (up_editorInputMap->IsActionPressed(EditorAction::CancelTransform)) {
            up_transformController->Cancel();
        } else {
            if (up_editorInputMap->IsActionPressed(EditorAction::AxisX)) up_transformController->HandleAxisKey(TransformAxis::X, p_camera->GetViewMatrix(), m_proj, pos.x, pos.y);
            else if (up_editorInputMap->IsActionPressed(EditorAction::AxisY)) up_transformController->HandleAxisKey(TransformAxis::Y, p_camera->GetViewMatrix(), m_proj, pos.x, pos.y);
            else if (up_editorInputMap->IsActionPressed(EditorAction::AxisZ)) up_transformController->HandleAxisKey(TransformAxis::Z, p_camera->GetViewMatrix(), m_proj, pos.x, pos.y);

            up_transformController->Apply(p_camera->GetViewMatrix(), m_proj, pos.x, pos.y);

            if (up_editorInputMap->IsActionPressed(EditorAction::ConfirmTransform)) {
                up_transformController->End();
            }
        }
    }
}

void Editor::RenderOverlays() {
    if (!up_ui->GetViewportInfo().visible) return;          // no 3D view on screen, skip every pass

    up_gridRenderer->Render(p_camera->GetPosition());

    if (up_selectionManager->GetActiveSelected().has_value()) {
        Entity active = up_selectionManager->GetActiveSelected().value();
        mathpp::vec3f medianPos = ComputeMedianPos();
        if (m_gizmoData.visible) {
            up_gizmo->Render(p_scene,medianPos,p_camera->GetPosition());

            up_gizmo->DrawOriginMarker(medianPos);
        }
        const mathpp::vec2f pos = LocalCursor();
        up_gizmoAdapter->OnMouseMove(static_cast<int>(pos.x), static_cast<int>(pos.y));
    }

    up_selector->RenderScene(p_scene,p_transformSystem,p_meshSystem);
    up_outline->Draw(p_scene);
}

void Editor::RunUI() {
    up_ui->BeginFrame();
    up_ui->RenderPanels();
    up_ui->RenderPrimitiveOp();
    up_ui->RenderViewportMode();
    up_ui->RenderAddMenu();
    up_ui->EndFrame();
}

void Editor::SyncViewportSize(int w, int h) {
    if (w == m_vpWidth && h == m_vpHeight) return;
    m_vpWidth = w;
    m_vpHeight = h;
    m_width = static_cast<float>(w);
    m_height = static_cast<float>(h);
    up_transformController->SetViewportSize(m_width, m_height);
}

mathpp::vec2f Editor::LocalCursor() const {
    const ViewportInfo& vp = up_ui->GetViewportInfo();
    mathpp::vec2f pos;
    p_input->GetCursorPos(pos);
    pos.x -= vp.x;
    pos.y -= vp.y;
    return pos;
}

void Editor::ShutDown() {
    up_ui->Shutdown();
    for (auto& i : v_handles) {
        i.first->Unsubscribe(i.second);
    }
}

Editor::Editor() = default;
Editor::~Editor() = default;

void Editor::TrySelect(int mx, int my) {
    if (m_mode == EditorMode::ObjectMode) {
        gill_editor::Entry entry;
        entry.f_callable =
        std::optional<Entity> picked = up_selector->ReadEntityAt(mx, my);
        if (!picked.has_value()) {
            up_selectionManager->ClearSelection();
            return;
        }



        if (p_input->IsShiftHeld()) {
            up_selectionManager->ToggleSelection(picked.value());
        } else {
            up_selectionManager->SetSelected(picked.value());
        }
    }
}




mathpp::vec3f Editor::ComputeMedianPos() {
  const auto& selected = up_selectionManager->GetAllSelected();
    float size = static_cast<float>(selected.size());
    mathpp::vec3f sum;
    for (auto& entity : selected) {
        mathpp::mat4f worldTransform = p_transformSystem->GetWorldTransform(entity);
        mathpp::vec3f worldPos = mathpp::TranslateFromMat4(worldTransform);
        sum += worldPos;
    }
    return sum / size;
}

void Editor::UpdateCursorForModalDrag() {
    // m_viewportActive is latched while a mouse button is down, so a camera drag that
    // started in another panel (e.g. the node editor) no longer hides the cursor
    bool modalDrag = up_transformController->IsDragging() || (m_viewportActive && p_camera->IsDragging(p_input));
    const int mode = modalDrag ? 2 : 0;
    if (mode != m_cursorMode) {                              // only talk to GLFW on change
        p_input->SetCursorMode(mode);
        m_cursorMode = mode;
    }
}

const ViewportInfo& Editor::GetViewportInfo() {
    return up_ui->GetViewportInfo();
}

Framebuffer &Editor::GetViewportFramebuffer() {
    return up_ui->GetViewportFramebuffer();
}


void Editor::SetProjection(const mathpp::mat4f& proj) {
    m_proj = proj;
}