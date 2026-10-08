#include "core/engine/engine.hpp"
#include <iostream>
#include <GLFW/glfw3.h>
#include "render/core/renderer.hpp"
#include "core/component/camera/camera.hpp"
#include "core/system/armature/armature.hpp"
#include "core/system/transform/transform.hpp"
#include "core/system/hierarchy/hierarchy.hpp"
#include "core/system/material/material.hpp"
#include "core/system/mesh/mesh.hpp"


void Engine::Init(unsigned int width, unsigned int height,Camera* camera,Scene* scene) {
    p_cam = camera;
    p_scene = scene;
    up_renderer = std::make_unique<Renderer>();
    up_transformSystem = std::make_unique<TransformSystem>();
    up_hierarchy = std::make_unique<Hierarchy>();
    up_meshSystem = std::make_unique<MeshSystem>();
    up_materialSystem = std::make_unique<MaterialSystem>();
    up_transformSystem->Init(up_hierarchy.get());
    up_armatureSystem = std::make_unique<ArmatureSystem>();
    up_renderer->Init(up_transformSystem.get(),up_meshSystem.get(),up_materialSystem.get(),up_armatureSystem.get());
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
