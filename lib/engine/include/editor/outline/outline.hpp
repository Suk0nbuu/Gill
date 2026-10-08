#pragma once
#include <memory>
#include <cstdint>
#include "mathpp.hpp"
#include "scene/scene.hpp"


using Entity = uint32_t;
class Shader;
class MeshSystem;
class TransformSystem;
class SelectionManager;

class Outline {
public:


    void Init(MeshSystem* meshSystem,TransformSystem* transformSystem,SelectionManager* selectionManager);
    void Draw(const Scene* scene) const ;
    ~Outline();


private:

    void DrawEntity(const Scene* scene,Entity entity,const comp::MeshComponent& meshComp) const ;
    MeshSystem* p_meshSystem;
    TransformSystem* p_transformSystem;
    SelectionManager* p_selectionManager;
    std::unique_ptr<Shader> up_outlineShader;


};