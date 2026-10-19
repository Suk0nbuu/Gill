#pragma once

class Scene;
class Hierarchy;
class TransformSystem;
class MeshSystem;
class MaterialSystem;
class ProjectPaths;
class ArmatureSystem;

struct EngineContext {
    Scene* p_scene = nullptr;
    Hierarchy* p_hierarchy = nullptr;
    TransformSystem* p_transformSystem = nullptr;
    MeshSystem* p_meshSystem = nullptr;
    MaterialSystem* p_materialSystem = nullptr;
    ProjectPaths* p_projectPaths = nullptr;
    ArmatureSystem* p_armatureSystem = nullptr;
};