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


void Editor::Init(float Width, float Height,Window* window,Scene* scene,Input* input,const mathpp::mat4f& projection, Camera* camera,TransformSystem* transformSystem,Hierarchy* hierarchy,MeshSystem* meshSystem, MaterialSystem* materialSystem,Renderer* renderer, ArmatureSystem* armatureSystem) {
    p_window = window;
    p_scene = scene;
    m_width = Width;
    m_height = Height;
    m_proj = projection;
    Shader shader("asset/shader/simpleShader/simpleVert.glsl", "asset/shader/simpleShader/simpleFrag.glsl");
    p_camera = camera;
    p_renderer = renderer;
    up_selector = std::make_unique<Selector>();
    up_gridRenderer = std::make_unique<GridRenderer>();
    up_transformController = std::make_unique<TransformController>();
    p_hierarchy = hierarchy;
    p_transformSystem = transformSystem;
    p_meshSystem = meshSystem;
    p_materialSystem = materialSystem;
    p_input = input;
    p_armatureSystem = armatureSystem;
    up_selectionManager = std::make_unique<SelectionManager>();
    up_editorInputMap = std::make_unique<EditorInputMap>(p_input);
    up_outline = std::make_unique<Outline>();
    up_ui = std::make_unique<UIManager>();
    up_gizmo = std::make_unique<Gizmo>();
    up_gizmoAdapter = std::make_unique<GizmoAdapter>();
    up_gridRenderer->Init(100);
    up_gizmo->Init(m_width,m_height,&m_gizmoData,p_transformSystem);
    up_ui->Init(window,p_scene,p_transformSystem,p_hierarchy,&m_gizmoData,p_renderer,p_meshSystem,p_materialSystem,up_editorInputMap.get(),up_selectionManager.get(),p_input);
    up_selector->Init(m_width,m_height);
    up_outline->Init(meshSystem,transformSystem,up_selectionManager.get());
    up_transformController->Init(m_width,m_height,&m_gizmoData,p_transformSystem,up_selectionManager.get(),hierarchy);
    up_gizmoAdapter->Init(up_gizmo.get(), up_transformController.get(), &m_gizmoData, up_selectionManager.get());

    auto handle1 = p_input->mouseDown.Subscribe([this](int mx, int my) {
        bool consumed = up_gizmoAdapter->OnMouseDown(mx, my, p_camera->GetViewMatrix(), m_proj);
        if (!consumed && !up_ui->WantCaptureMouse()) {
            TrySelect(mx, my);
        }
    });
    auto handle2 = p_input->mouseUp.Subscribe([this](int mx, int my) {
        up_gizmoAdapter->OnMouseUp();
    });

    v_handles.push_back(std::make_pair(&p_input->mouseDown,handle1));
    v_handles.push_back(std::make_pair(&p_input->mouseUp,handle2));
    TestFunction(scene,meshSystem,transformSystem,armatureSystem);
}

void Editor::Run(float deltaT) {
    up_gridRenderer->Render(p_camera->GetViewMatrix(),m_proj,p_camera->GetPosition());
    mathpp::vec2f pos;
    p_input->GetCursorPos(pos);
    UpdateCursorForModalDrag();



    if (up_selectionManager->GetActiveSelected().has_value()) {
        Entity active = up_selectionManager->GetActiveSelected().value();
        mathpp::vec3f medianPos = ComputeMedianPos();
        if (m_gizmoData.visible) {
            up_gizmo->Render(p_scene,p_camera->GetViewMatrix(),m_proj,medianPos,p_camera->GetPosition(),active);
            up_gizmo->RenderIDs(p_camera->GetViewMatrix(),m_proj,medianPos,p_camera->GetPosition(),active);
            up_gizmo->DrawOriginMarker(p_camera->GetViewMatrix(),m_proj,medianPos);
        }
        up_gizmoAdapter->OnMouseMove(static_cast<int>(pos.x), static_cast<int>(pos.y));
    }

    if (!up_transformController->IsDragging()) {
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

    up_selector->RenderScene(p_scene,p_camera->GetViewMatrix(),m_proj,p_transformSystem,p_meshSystem);
    up_outline->Draw(p_scene,m_proj,p_camera->GetViewMatrix());
    up_ui->BeginFrame();
    up_ui->RenderPanels();
    up_ui->RenderPrimitiveOp(p_scene);
    up_ui->RenderViewportMode();
    up_ui->RenderAddMenu(p_scene);
    up_ui->EndFrame();
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
    bool modalDrag = up_transformController->IsDragging() || p_camera->IsDragging(p_input);
    if (modalDrag) {
        p_input->SetCursorMode(2);
    } else {
        p_input->SetCursorMode(0);
    }
}