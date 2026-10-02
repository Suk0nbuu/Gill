#include "core/debug/test.hpp"
#include "io/gltf/gltf.hpp"
#include <iostream>
#include "scene/scene.hpp"
#include "core/system/mesh/mesh.hpp"
#include "core/system/transform/transform.hpp"
#include "core/system/armature/armature.hpp"
void TestFunction(Scene* scene,MeshSystem* meshSystem,TransformSystem* transformSystem,ArmatureSystem* armatureSystem) {
    GLTFModel model = LoadGLTF("asset/mesh/test/TestFile.glb");
    bool hasArm = model.armature.has_value();
    Entity entity = scene->CreateEntity();
    transformSystem->AddTransform(entity);
    comp::MeshComponent meshComp;
    meshComp.meshID = meshSystem->RegisterMesh(std::move(model.meshes[0]));

    scene->InsertComponent(entity, meshComp);

    if (model.armature.has_value()) {
        comp::ArmatureComponent armComp;
        armComp.armature = model.armature.value();
        armatureSystem->ResetToBindPose(armComp);
        scene->InsertComponent(entity, armComp);
    }



}