#include "glad/gl.h"
#include "editor/outline/outline.hpp"
#include "core/system/mesh/mesh.hpp"
#include "core/system/transform/transform.hpp"
#include "editor/selector/selectionManager.hpp"
#include "render/shader/shader.hpp"



void Outline::Init(MeshSystem* meshSystem,TransformSystem* transformSystem,SelectionManager* selectionManager) {
    up_outlineShader = std::make_unique<Shader>("asset/shader/outlineShader/outlineVert.glsl","asset/shader/outlineShader/outlineFrag.glsl");
    p_meshSystem = meshSystem;
    p_transformSystem = transformSystem;
    p_selectionManager = selectionManager;
}

void Outline::Draw(const Scene* scene) const {
    up_outlineShader->Use();
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    scene->ForEach<comp::MeshComponent>([this,scene](Entity entity,const comp::MeshComponent& meshComp) {
        DrawEntity(scene,entity,meshComp);
    });
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
}

void Outline::DrawEntity(const Scene* scene,Entity entity,const comp::MeshComponent& meshComp) const {
    const  auto& selected = p_selectionManager->GetAllSelected();
    if (!selected.contains(entity)) return; //safeguard
    auto mesh = p_meshSystem->GetMesh(meshComp.meshID);
    auto worldTransform = p_transformSystem->GetWorldTransform(entity);
    if (!mesh) return;

    up_outlineShader->setMat4f("model",worldTransform);
    mesh->Draw();
}

Outline::~Outline() = default;