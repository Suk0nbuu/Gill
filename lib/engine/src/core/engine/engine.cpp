#include "core/engine/engine.hpp"
#include <iostream>
#include <GLFW/glfw3.h>
#include "render/core/renderer.hpp"
#include "core/component/camera/camera.hpp"
#include "core/engine/engineContext.hpp"
#include "core/system/armature/armature.hpp"
#include "core/system/transform/transform.hpp"
#include "core/system/hierarchy/hierarchy.hpp"
#include "core/system/material/material.hpp"
#include "core/system/mesh/mesh.hpp"
#include "core/project/projectPath.hpp"


void Engine::Init(unsigned int width, unsigned int height,Camera* camera,Scene* scene) {
    p_cam = camera;
    p_scene = scene;
    up_projectPaths = std::make_unique<ProjectPaths>();
    up_renderer = std::make_unique<Renderer>();
    up_transformSystem = std::make_unique<TransformSystem>();
    up_hierarchy = std::make_unique<Hierarchy>();
    up_meshSystem = std::make_unique<MeshSystem>();
    up_materialSystem = std::make_unique<MaterialSystem>(up_projectPaths.get());
    up_transformSystem->Init(up_hierarchy.get());
    up_armatureSystem = std::make_unique<ArmatureSystem>();
    m_engineContext.p_projectPaths = up_projectPaths.get();
    m_engineContext.p_scene = scene;
    m_engineContext.p_materialSystem = up_materialSystem.get();
    m_engineContext.p_meshSystem = up_meshSystem.get();
    m_engineContext.p_hierarchy = up_hierarchy.get();
    m_engineContext.p_transformSystem = up_transformSystem.get();

    up_renderer->Init(scene,up_transformSystem.get(),up_meshSystem.get(),up_materialSystem.get(),up_armatureSystem.get(),up_hierarchy.get());
    m_width = width;
    m_height = height;

}

void Engine::Run() {
    up_renderer->renderScene(p_scene,p_cam->GetViewMatrix(),m_proj,p_cam->GetPosition());
}

void Engine::Shutdown() {
    glfwTerminate();
}


Engine::Engine(const mathpp::mat4f& projection)
    : m_proj(const_cast<mathpp::mat4f&>(projection))
{
}

Engine::~Engine() = default;

TransformSystem *Engine::GetTransformSystem() {
    return up_transformSystem.get();
}

Hierarchy *Engine::GetHierarchy() {
    return up_hierarchy.get();
}

ArmatureSystem *Engine::GetArmatureSystem() {
    return up_armatureSystem.get();
}

MaterialSystem *Engine::GetMaterialSystem() {
    return up_materialSystem.get();
}

MeshSystem *Engine::GetMeshSystem() {
    return up_meshSystem.get();
}

Renderer *Engine::GetRenderer() {
    return up_renderer.get();
}

ProjectPaths *Engine::GetProjectPath() {
    return up_projectPaths.get();
}

EngineContext& Engine::Context() {
    return m_engineContext;
}

void Engine::SetProjection(const mathpp::mat4f &proj) {
    m_proj = proj;
}
