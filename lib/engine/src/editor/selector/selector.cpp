#include "editor/selector/selector.hpp"
#include "component/mesh.hpp"
#include "core/system/mesh/mesh.hpp"
#include "render/shader/shader.hpp"
#include "scene/scene.hpp"
#include "core/system/transform/transform.hpp"
#include "editor/core/fboScope.hpp"
#include <algorithm>

#include "editor/picker/pickerData.hpp"


void Selector::Init() {
    up_selectShader = std::make_unique<Shader>("asset/shader/selectShader/selectVert.glsl","asset/shader/selectShader/selectFrag.glsl");
}



void Selector::RenderScene(const Scene* scene,TransformSystem* transformSystem,MeshSystem* meshSystem) {
    up_selectShader->Use();
    scene->ForEach<comp::MeshComponent>([this, scene,transformSystem,meshSystem](Entity entity,const comp::MeshComponent& meshComp) {
       RenderEntityID(scene, entity, meshComp,transformSystem,meshSystem);
   });
}



void Selector::RenderEntityID(const Scene *scene,Entity entity, const comp::MeshComponent &meshComp,TransformSystem* transformSystem,MeshSystem* meshSystem) {
    auto mesh = meshSystem->GetMesh(meshComp.meshID);
    auto id = gill_editor::Encode(gill_editor::Type::Entity,entity);
    up_selectShader->SetMat4f("model",transformSystem->GetWorldTransform(entity) );
    up_selectShader->SetUInt("ObjectID", static_cast<int>(id));
    mesh->Draw();


}


