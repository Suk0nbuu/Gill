#include "scene/sceneSerializer.hpp"
#include "core/system/hierarchy/hierarchy.hpp"
#include "core/system/transform/transform.hpp"
#include "magic_enum.hpp"
#include "core/system/mesh/mesh.hpp"
#include <algorithm>
#include "nlohmann/json.hpp"
#include <unordered_map>
#include <fstream>

using json = nlohmann::json;

namespace {
    constexpr int SUPPORTED_VERSION = 1;
    constexpr int NO_PARENT = -1;
    template <typename T>
   static bool Read(const json& obj, const char* key, T& out, std::string& error) {
        if (!obj.is_object() || !obj.contains(key)) {
            error = std::string("'") + key + "' is missing";
            return false;
        }
        const json& v = obj.at(key);
        bool ok = false;
        if constexpr (std::is_same_v<T, std::string>)       ok = v.is_string();
        else if constexpr (std::is_integral_v<T>)           ok = v.is_number_integer();
        else if constexpr (std::is_floating_point_v<T>)     ok = v.is_number();

        if (ok) out = v.get<T>();
        else    error = std::string("'") + key + "' has the wrong type";
        return ok;
    }
    template<size_t N>
bool ReadFloats(const json& obj, const char* key, float (&out)[N], std::string& error) {
        if (!obj.is_object() || !obj.contains(key)) {
            error = std::string("missing key: ") + key;
            return false;
        }
        const json& arr = obj[key];            // obj is const, so operator[] is safe
        if (!arr.is_array() || arr.size() != N) {
            error = std::string(key) + " must be an array of " + std::to_string(N) + " numbers";
            return false;
        }
        float tmp[N];
        for (size_t i = 0; i < N; ++i) {
            if (!arr[i].is_number()) {         // is_number, not is_number_float: [1,1,1] is valid
                error = std::string(key) + "[" + std::to_string(i) + "] is not a number";
                return false;
            }
            tmp[i] = arr[i].get<float>();
        }
        std::copy(tmp, tmp + N, out);          // only reached if every element passed
        return true;
    }

    bool ParseEntity(const json& entity, Entry& out, std::string& error) {
    if (!entity.is_object()) {
        error = "entity is not an object";
        return false;
    }

    Entry local{};

    // id / parent
    if (!Read(entity, "id", local.id, error)) return false;
    if (local.id < 0) { error = "id must be >= 0"; return false; }

    if (!Read(entity, "parent", local.parent, error)) return false;
    if (local.parent < NO_PARENT)        { error = "parent must be >= -1"; return false; }
    if (local.parent == local.id) { error = "entity cannot be its own parent"; return false; }

    // transform
    if (!entity.contains("transform") || !entity["transform"].is_object()) {
        error = "missing or invalid 'transform' object";
        return false;
    }
    const json& transform = entity["transform"];

    float pos[3], rot[4], scale[3];
    if (!ReadFloats(transform, "pos",   pos,   error)) return false;
    if (!ReadFloats(transform, "rot",   rot,   error)) return false;
    if (!ReadFloats(transform, "scale", scale, error)) return false;

    // rot is stored w,x,y,z; reject degenerate quaternions, normalize the rest
    float len = std::sqrt(rot[0]*rot[0] + rot[1]*rot[1] + rot[2]*rot[2] + rot[3]*rot[3]);
    if (!std::isfinite(len) || len < 1e-6f) {
        error = "rot has zero or invalid length";
        return false;
    }
    local.rot.w = rot[0] / len;
    local.rot.x = rot[1] / len;
    local.rot.y = rot[2] / len;
    local.rot.z = rot[3] / len;

    local.pos.x = pos[0];  local.pos.y = pos[1];  local.pos.z = pos[2];
    local.scale.x = scale[0]; local.scale.y = scale[1]; local.scale.z = scale[2];

    // primitive
    if (!entity.contains("primitive") || !entity["primitive"].is_object()) {
        error = "missing or invalid 'primitive' object";
        return false;
    }
    const json& prim = entity["primitive"];

    std::string typeName;
    if (!Read(prim, "type", typeName, error)) return false;
    auto type = magic_enum::enum_cast<PrimitiveType>(typeName);
    if (!type) { error = "unknown primitive type: " + typeName; return false; }
    local.prim.type = *type;

    if (!Read(prim, "rings",    local.prim.rings,    error)) return false;
    if (!Read(prim, "segments", local.prim.segments, error)) return false;
    if (!Read(prim, "radius",   local.prim.radius,   error)) return false;
    if (!Read(prim, "height",   local.prim.height,   error)) return false;

    if (local.prim.rings < 2 || local.prim.rings > 128)       { error = "rings out of range (2-128)"; return false; }
    if (local.prim.segments < 3 || local.prim.segments > 128) { error = "segments out of range (3-128)"; return false; }
    if (!(local.prim.radius > 0.0f) || local.prim.radius > 100.0f) { error = "radius out of range (0-100]"; return false; }
    if (!(local.prim.height > 0.0f) || local.prim.height > 100.0f) { error = "height out of range (0-100]"; return false; }

    out = local;
    return true;
}
}



SceneSerializer::SceneSerializer(Scene *scene, TransformSystem *transformSystem, Hierarchy *hierarchy, MeshSystem *meshSystem) {
    p_hierarchy = hierarchy;
    p_transformSystem = transformSystem;
    p_meshSystem = meshSystem;
    p_scene = scene;
}

bool SceneSerializer::Parse(const std::string& text, std::vector<Entry>& out, std::string& error) {

    json root = json::parse(text, nullptr, false);
    if (root.is_discarded() || !root.is_object()) {
        error = "...";
        return false;
    }

    int version = 0;
    if (!Read(root, "version", version, error)) return false;
    if (version < 1 || version > SUPPORTED_VERSION) {
        error = "version not supported: " + std::to_string(version);
        return false;
    }


    if (!root.contains("entities")||!root["entities"].is_array()) {
        error = "missing entities array";
        return false;
    }
    const json& entities = root["entities"];   // root is non-const here, so only do this after contains()
    if (entities.size()>MAX_ENTITIES) {
        error = "entities exceeding max entity limit" + std::to_string(MAX_ENTITIES);
        return false;
    }


    std::vector<Entry> local;
    local.reserve(entities.size());

    for (size_t i = 0; i < entities.size(); ++i) {
        Entry entry{};
        if (!ParseEntity(entities[i], entry, error)) {
            error = "entities[" + std::to_string(i) + "]: " + error;
            return false;
        }
        local.push_back(entry);
    }

    constexpr int NO_PARENT = -1;

    std::unordered_map<int, size_t> index;
    for (size_t i = 0; i < local.size(); ++i)
        if (!index.emplace(local[i].id, i).second) {
            error = "duplicate id " + std::to_string(local[i].id);
            return false;
        }

    for (const Entry& e : local) {
        if (e.parent == NO_PARENT) continue;           // top-level, nothing to look up
        if (!index.contains(e.parent)) {
            error = "entity " + std::to_string(e.id) + " has unknown parent " + std::to_string(e.parent);
            return false;
        }
    }

    for (const Entry& e : local) {
        int cur = e.id;
        for (size_t steps = 0; cur != NO_PARENT; ++steps) {
            if (steps > local.size()) {
                error = "parent cycle involving entity " + std::to_string(e.id);
                return false;
            }
            cur = local[index.at(cur)].parent;
        }
    }
    out = std::move(local);
    return true;
}


void SceneSerializer::Apply(const std::vector<Entry> &entries) {
    std::unordered_map<int, Entity> idToEntity;
    for (size_t i = 0; i < entries.size(); ++i) {
        const Entry& e = entries[i];
        Entity ent = p_scene->CreateEntity();
        idToEntity.emplace(e.id, ent);
    }

    for (size_t i = 0; i < entries.size(); ++i) {
        const Entry& e = entries[i];
        if (e.parent == NO_PARENT) continue;
        p_transformSystem->AddTransform(idToEntity[e.id]);
        p_transformSystem->SetPosition(idToEntity[e.id],e.pos);
        p_transformSystem->SetRotation(idToEntity[e.id],e.rot);
        p_transformSystem->SetScale(idToEntity[e.id],e.scale);

        comp::PrimitiveComponent primComp;
        comp::MeshComponent meshComp;

        primComp.height = e.prim.height;
        primComp.radius = e.prim.radius;
        primComp.rings = e.prim.rings;
        primComp.segments = e.prim.segments;
        primComp.type = e.prim.type;

        meshComp.meshID = p_meshSystem->AddPrimitive(primComp.type,primComp.rings,primComp.segments,primComp.radius,primComp.height);
        p_scene->InsertComponent(idToEntity[e.id],meshComp);
        p_scene->InsertComponent(idToEntity[e.id],primComp);
        p_hierarchy->SetParent(idToEntity[e.id], idToEntity[e.parent]);
    }
}


bool SceneSerializer::Load(const std::filesystem::path &path, std::string &error) {
    std::vector<Entry> entries;
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        error = "failed to open " + path.string();
        return false;
    }
    const json& root = json::parse(ifs);
    auto str = root.dump();

    if (!Parse(str,entries,error)) {
        return false;
    }
    Apply(entries);
    return true;
}

bool SceneSerializer::Save(const std::filesystem::path &path, std::string &error, int &skipped) const {
    std::ofstream ofs(path);
    const json root;
    root["version"] = SUPPORTED_VERSION;


    if (!ofs.is_open()) {
        error = "failed to open " + path.string();
    }
    std::vector<Entity> entities = p_scene->GetLivingEntities();
    for (size_t i = 0; i < entities.size(); ++i) {
        const Entity& e = entities[i];

    }
}
