#pragma once

class Scene;
class Hierarchy;
class TransformSystem;
class MeshSystem;
class MaterialSystem;
class ProjectPaths;

struct EngineContext {
    Scene* p_scene;
    Hierarchy* p_hierarchy;
    TransformSystem* p_transformSystem;
    MeshSystem* p_meshSystem;
    MaterialSystem* p_materialSystem;
    ProjectPaths* p_projectPaths;
};