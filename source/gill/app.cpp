#include "app.hpp"
#include <glad/gl.h>
#include "../../lib/engine/include/core/window/window.hpp"
#include "core/component/camera/camera.hpp"
#include "core/engine/engine.hpp"
#include "editor/core/editor.hpp"
#include "scene/scene.hpp"
#include "core/input/input.hpp"

void App::Init(unsigned int width, unsigned int height) {
    int actualWidth, actualHeight;
    up_window = std::make_unique<Window>(width,height,title);
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
    up_editor->Init(m_width,m_height,up_window.get(),up_scene.get(),up_input.get(),m_projection,up_camera.get(),up_engine->GetTransformSystem(),up_engine->GetHierarchy(),up_engine->GetMeshSystem(),up_engine->GetMaterialSystem(),up_engine->GetRenderer());
}


void App::Run() {
    while (!up_window->ShouldClose()) {
        float currentTime = glfwGetTime();
        m_deltaTime = currentTime - m_lastFrame;
        m_lastFrame = currentTime;
        up_window->PollEvents();
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        up_input->Update();
        up_camera->Update(up_input.get(),m_deltaTime,{0.0f,0.0f,0.0f});
        up_engine->Run();
        up_editor->Run(m_deltaTime);
        up_input->ClearFrameState();
        up_window->SwapBuffers();
    }
}

void App::Shutdown() {
    up_editor->ShutDown();
    up_engine->Shutdown();
    glfwTerminate();
}

App::App() = default;
App::~App() = default;
