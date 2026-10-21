#include "app.hpp"
#include <glad/gl.h>
#include "../../lib/engine/include/core/window/window.hpp"
#include "core/component/camera/camera.hpp"
#include "core/engine/engine.hpp"
#include "editor/core/editor.hpp"
#include "scene/scene.hpp"
#include "core/input/input.hpp"
#include "../../launcher/include/core/launcher.hpp"
#include "nfd.h"
#include "../../engine/include/scene/sceneSerializer.hpp"
#include "core/project/projectPath.hpp"

bool App::Init(unsigned int width, unsigned int height) {
    int actualWidth, actualHeight;
    NFD_Init();
    up_window = std::make_unique<Window>(width,height,title);

    Launcher launcher;
    m_project = launcher.Run(*up_window);
    if (!m_project) return false;
    glfwSetWindowTitle(up_window->GetWindow(), (title + " - " + m_project->GetName()).c_str());

    up_window->GetFrameBufferSize(&actualWidth, &actualHeight);

    m_width = static_cast<float>(actualWidth);
    m_height = static_cast<float>(actualHeight);
    m_aspect = static_cast<float>(width) / static_cast<float>(height);
    m_projection = mathpp::perspective(45.0f,m_aspect,0.01f,300.0f);

    up_engine = std::make_unique<Engine>(m_projection);
    up_editor = std::make_unique<Editor>();
    up_scene = std::make_unique<Scene>();
    up_camera = std::make_unique<Camera>();
    up_input = std::make_unique<Input>(up_window.get());
    up_engine->Init(static_cast<int>(m_width),static_cast<int>(m_height),up_camera.get(),up_scene.get());
    up_editor->Init(m_width,m_height,up_window.get(),up_scene.get(),up_input.get(),m_projection,up_camera.get(),up_engine->GetTransformSystem(),up_engine->GetHierarchy(),up_engine->GetMeshSystem(),up_engine->GetMaterialSystem(),up_engine->GetRenderer(),up_engine->GetArmatureSystem(),up_engine->GetProjectPath());
    up_engine->GetProjectPath()->SetRoot(m_project->GetRoot());
    std::string err;
    if (!LoadScene(err)) { std::fprintf(stderr, "Scene load failed: %s\n", err.c_str()); return false; }
    up_editor->saveRequested.Subscribe([this]{ SaveScene(); });
    up_editor->closeRequested.Subscribe([this]{ m_closeRequested = true; });   //
    return true;
}

#include "render/framebuffer/framebuffer.hpp"
#include "editor/ui/viewportInfo.hpp"

void App::Run() {
    while (!up_window->ShouldClose() && !m_closeRequested) {
        float currentTime = glfwGetTime();
        m_deltaTime = currentTime - m_lastFrame;
        m_lastFrame = currentTime;
        up_window->PollEvents();

        int winW = 0, winH = 0;
        up_window->GetFrameBufferSize(&winW, &winH);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);       // the window itself now only shows the UI

        const ViewportInfo& vp = up_editor->GetViewportInfo();

        up_editor->BeginFrameInput();                              // decides who owns the mouse this frame; must run before Input::Update fires its events
        up_input->Update();

        Framebuffer& fb = up_editor->GetViewportFramebuffer();
        if (vp.visible) {
            fb.Resize(vp.width, vp.height);
            UpdateProjection(static_cast<float>(vp.width) / static_cast<float>(vp.height));
        } else {
            fb.Resize(1, 1);
        }
        up_editor->Update(m_deltaTime);                            // camera, input and modal transforms

        fb.Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        up_engine->Run();
        up_editor->RenderOverlays();                               // grid, gizmo, picking, outline: into the viewport image
        Framebuffer::Unbind(winW, winH);

        up_editor->RunUI();
        up_input->ClearFrameState();
        up_window->SwapBuffers();
    }
}

void App::Shutdown() {
    up_editor->ShutDown();
    up_engine->Shutdown();
    glfwTerminate();
    NFD_Quit();
}

App::App() = default;
App::~App() = default;

bool App::LoadScene(std::string& error) {
    SceneSerializer ser(up_engine->Context());
    return ser.Load(m_project->ScenePath(), error);
}

void App::UpdateProjection(float aspect) {
    if (std::abs(aspect - m_aspect) < 1e-4f) return;
    m_aspect = aspect;
    m_projection = mathpp::perspective(45.0f, m_aspect, 0.01f, 300.0f);
    up_engine->SetProjection(m_projection);
    up_editor->SetProjection(m_projection);
}

void App::SaveScene() {
    SceneSerializer ser(up_engine->Context());
    std::string err; int skipped = 0;
    if (!ser.Save(m_project->ScenePath(), err, skipped)) { m_errorPopup = "Save failed: " + err; return; }
    m_dirty = false;
}