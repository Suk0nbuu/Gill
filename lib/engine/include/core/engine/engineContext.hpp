#pragma once

class Scene;
class Hierarchy;
class TransformSystem;
class MeshSystem;
class MaterialSystem;

struct EngineContext {
    Scene*           p_scene;
    Hierarchy*       p_hierarchy;
    TransformSystem* p_transformSystem;
    MeshSystem*      p_meshSystem;
    MaterialSystem*  p_materialSystem;
};