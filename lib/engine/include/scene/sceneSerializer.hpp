#pragma once


#include <filesystem>
#include <string>
#include <vector>
#include "mathpp.hpp"


#include "scene/scene.hpp"
class TransformSystem;
class Hierarchy;
class MeshSystem;

struct Entry { int id; int parent; mathpp::vec3f pos; mathpp::quatf rot; mathpp::vec3f scale; comp::PrimitiveComponent prim; };



class SceneSerializer {
public:
    SceneSerializer(Scene* scene, TransformSystem* transforms, Hierarchy* hierarchy, MeshSystem* meshes);


    bool Save(const std::filesystem::path& path, std::string& error, int& skipped) const;
    bool Load(const std::filesystem::path& path, std::string& error);

private:

    static bool Parse(const std::string& text, std::vector<Entry>& out, std::string& error);
    void Apply(const std::vector<Entry>& entries);



    Scene*           p_scene;
    TransformSystem* p_transformSystem;
    Hierarchy*       p_hierarchy;
    MeshSystem*      p_meshSystem;
};