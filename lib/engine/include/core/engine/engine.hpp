#pragma once
#include <string>
#include <memory>
#include "mathpp.hpp"



class Renderer ;
class Camera;
class Scene;
class TransformSystem;
class Hierarchy;
class MeshSystem;
class MaterialSystem;
class ArmatureSystem;

class Engine {

public:
    Engine(const mathpp::mat4f& projection);
    ~Engine();
    void Init(unsigned int width, unsigned int height,Camera* cam,Scene* scene);
    void Run();
    void Shutdown();
    TransformSystem* GetTransformSystem();
    Hierarchy* GetHierarchy();
    MeshSystem* GetMeshSystem();
    MaterialSystem* GetMaterialSystem();
    Renderer* GetRenderer();
    ArmatureSystem* GetArmatureSystem();

private:
    unsigned int m_width,m_height;
    std::unique_ptr<Renderer> up_renderer;
    std::unique_ptr<TransformSystem> up_transformSystem;
    std::unique_ptr<Hierarchy> up_hierarchy;
    std::unique_ptr<MaterialSystem> up_materialSystem;
    std::unique_ptr<MeshSystem> up_meshSystem;
    std::unique_ptr<ArmatureSystem> up_armatureSystem;
    Camera* p_cam;
    Scene* p_scene;

    mathpp::mat4f& m_proj;

};
