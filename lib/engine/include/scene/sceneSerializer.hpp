#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <utility>
#include "mathpp.hpp"
#include "scene/scene.hpp"
#include "core/engine/engineContext.hpp"
class TransformSystem;
class Hierarchy;
class MeshSystem;
class MaterialSystem;
struct MaterialData {
    std::string shader = "builtin:brdf";
    std::string texture;
    std::vector<std::pair<std::string, PropValue>> props;
    std::optional<ShaderGraph> graph;
};


struct Entry {
    int id;
    int parent;
    std::string name;
    mathpp::vec3f pos;
    mathpp::quatf rot;
    mathpp::vec3f scale;
    comp::PrimitiveComponent prim;
    comp::LightComponent light;
    comp::MeshRefComponent meshRef;
    MaterialData mat;
    bool hasPrimitive = false;
    bool hasMaterial = false;
    bool hasLight = false;
    bool hasMeshRef = false;

};


struct EngineContext;

class SceneSerializer {
public:
    SceneSerializer(EngineContext context);


    bool Save(const std::filesystem::path& path, std::string& error, int& skipped) const;
    bool Load(const std::filesystem::path& path, std::string& error);

private:

    static bool Parse(const std::string& text, std::vector<Entry>& out, std::string& error);
    bool Apply(const std::vector<Entry>& entries,std::string& error);



   EngineContext m_ctx;
};