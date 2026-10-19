#include "scene/sceneSerializer.hpp"
#include "core/system/hierarchy/hierarchy.hpp"
#include "core/system/transform/transform.hpp"
#include "core/system/material/material.hpp"
#include "magic_enum.hpp"
#include "core/system/mesh/mesh.hpp"
#include <algorithm>
#include "nlohmann/json.hpp"
#include "core/project/projectPath.hpp"
#include "io/gltf/gltf.hpp"
#include <unordered_map>
#include <fstream>
#include <cmath>

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

    bool ReadOptionalString(const json& obj, const char* key, std::string& out, std::string& error) {
        auto it = obj.find(key);
        if (it == obj.end() || it->is_null()) return true;          // missing/null = leave default
        if (!it->is_string()) { error = std::string("'") + key + "' must be a string or null"; return false; }
        out = it->get<std::string>();
        return true;
    }

    bool IsSafeRelativePath(const std::string& s) {
        std::filesystem::path p(s);
        if (p.is_absolute() || p.has_root_name()) return false;
        for (const auto& part : p) if (part == "..") return false;
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

        if (entity.contains("name")) { Read(entity, "name", local.name, error); }

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

    if (entity.contains("primitive") && entity["primitive"].is_object())  {
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
        local.hasPrimitive = true;
    }
    else {
        local.hasPrimitive = false;
        }

        if (entity.contains("material")) {
            if (!entity["material"].is_object()) { error = "'material' must be an object"; return false; }
            const json& m = entity["material"];
            MaterialData d;
            if (m.contains("props")) {
                if (!m["props"].is_object()) {error = "prop must be an object"; return false;}
                for (auto& [name,v]:m["props"].items()) {
                    if (v.is_number()) {
                        d.props.emplace_back(name,v.get<float>());
                    }
                    else if (v.is_string()) {
                        const std::string path = v.get<std::string>();
                        if (path.find('\\') != std::string::npos) { error = "texture path must use forward slashes"; return false; }
                        if (!path.empty() && !IsSafeRelativePath(path)) { error = "texture path is not safe: " + path; return false; }
                        d.props.emplace_back(name, TextureRef{path, AssetHandle{}});   // handle is resolved later, in Apply
                    }
                    else if (v.is_null()) {
                        d.props.emplace_back(name, TextureRef{});                       // no texture
                    }
                    else if (v.is_array() && v.size() == 3 && std::all_of(v.begin(), v.end(), [](const json& x){ return x.is_number(); })) {
                        d.props.emplace_back(name, mathpp::vec3f{v[0].get<float>(), v[1].get<float>(), v[2].get<float>()});
                    } else { error = "property '" + name + "' prop value must be a number, [r,g,b], a path string, or null "; return false; }
                }
            }

            else {
                float albedo[3], roughness, metallic;
                if (!ReadFloats(m,"albedo",  albedo,  error)|| !Read(m,"metallic",metallic,error)|| !Read(m,"roughness",roughness,error)) return false;
                d.props.emplace_back("albedo",mathpp::vec3f(albedo[0],albedo[1],albedo[2]));
                d.props.emplace_back("roughness",roughness);
                d.props.emplace_back("metallic",metallic);
            }


            if (!ReadOptionalString(m, "texture", d.texture, error)) return false;
            if (!ReadOptionalString(m, "shader",  d.shader,  error)) return false;
            if (!d.texture.empty() && !IsSafeRelativePath(d.texture)) { error = "texture path must be relative and stay inside the project"; return false; }
            if (d.shader.rfind("builtin:", 0) != 0 && !IsSafeRelativePath(d.shader)) { error = "shader must be 'builtin:<name>' or a safe relative path"; return false; }

            local.hasMaterial = true;
            local.mat = d;
        }
        if (entity.contains("light")) {
            if (!entity["light"].is_object()) { error = "'light' must be an object"; return false; }
            const json& l = entity["light"];
            comp::LightComponent d;

            std::string typeName;
            if (!Read(l, "type", typeName, error)) return false;
            auto type = magic_enum::enum_cast<comp::LightType>(typeName);
            if (!type) { error = "unknown light type: " + typeName; return false; }
            d.type = *type;

            float color[3];
            if (!ReadFloats(l, "color", color, error)) return false;
            if (!Read(l, "intensity", d.intensity, error)) return false;
            if (!Read(l, "range",     d.range,     error)) return false;
            if (!Read(l, "spotSize",  d.spotSize,  error)) return false;
            if (!Read(l, "blend",     d.blend,     error)) return false;

            for (float c : color) if (!(c >= 0.f && c <= 16.f)) { error = "light color out of range [0,16]"; return false; }
            if (!(d.intensity >= 0.f && d.intensity <= 1000.f)) { error = "intensity out of range [0,1000]"; return false; }
            if (!(d.range > 0.f && d.range <= 1000.f))          { error = "range out of range (0,1000]";   return false; }
            if (!(d.spotSize > 0.f && d.spotSize <= 180.f))     { error = "spotSize out of range (0,180]"; return false; }
            if (!(d.blend >= 0.f && d.blend <= 1.f))            { error = "blend out of range [0,1]";      return false; }

            d.color = {color[0], color[1], color[2]};
            local.light = d;
            local.hasLight = true;
        }
        if (entity.contains("meshRef")) {
            if (!entity["meshRef"].is_object()) { error = "'meshRef' must be an object"; return false; }
            if (local.hasPrimitive) { error = "entity has both 'primitive' and 'meshRef'"; return false; }
            const json& r = entity["meshRef"];
            if (!Read(r, "path", local.meshRef.path, error)) return false;
            if (!Read(r, "index", local.meshRef.index, error)) return false;
            const std::string& p = local.meshRef.path;
            if (p.empty() || p.find('\\') != std::string::npos || !IsSafeRelativePath(p)) { error = "meshRef path is not safe"; return false; }
            if (local.meshRef.index < 0 || local.meshRef.index > 4096) { error = "meshRef index out of range"; return false; }
            local.hasMeshRef = true;
        }

    out = local;
    return true;
}
}




SceneSerializer::SceneSerializer(EngineContext context) {
    m_ctx = context;
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


bool SceneSerializer::Apply(const std::vector<Entry> &entries,std::string& error) {
    std::unordered_map<int, Entity> idToEntity;
    idToEntity.reserve(entries.size());
    for (size_t i = 0; i < entries.size(); ++i) {
        const Entry& e = entries[i];
        Entity ent = m_ctx.p_scene->CreateEntity();
        idToEntity.emplace(e.id, ent);
        m_ctx.p_transformSystem->AddTransform(ent);
        m_ctx.p_transformSystem->SetPosition(ent,e.pos);
        m_ctx.p_transformSystem->SetRotation(ent,e.rot);
        m_ctx.p_transformSystem->SetScale(ent,e.scale);

        if (e.hasPrimitive == true) {
            comp::MeshComponent meshComp;
            meshComp.meshID = m_ctx.p_meshSystem->AddPrimitive(e.prim.type,e.prim.rings,e.prim.segments,e.prim.radius,e.prim.height);
            m_ctx.p_scene->InsertComponent(ent,meshComp);
            m_ctx.p_scene->InsertComponent(ent,e.prim);
        }
        if (e.hasMaterial) {
            Material mat = m_ctx.p_materialSystem->MakeMaterial(e.mat.shader);
            if (!m_ctx.p_materialSystem->GetShaderDef(e.mat.shader)) { error = "unsupported shader: " + e.mat.shader; return false; }
            for (const auto& [name, value] : e.mat.props)
                for (auto& p : mat.props)
                    if (p.name == name && p.value.index() == value.index()) { p.value = value; break; }

            for (auto& p : mat.props)
                if (auto* t = std::get_if<TextureRef>(&p.value); t && !t->path.empty())
                    t->handle = m_ctx.p_materialSystem->GetOrLoadTexture(t->path);
            comp::MaterialComponent mc;
            mc.materialID = m_ctx.p_materialSystem->RegisterMaterial(std::move(mat));
            m_ctx.p_scene->InsertComponent(ent, mc);
        }
        if (e.hasLight) m_ctx.p_scene->InsertComponent(ent, e.light);

        if (e.hasMeshRef) {
            const auto full = m_ctx.p_projectPaths->Root() / e.meshRef.path;
            GLTFModel model = LoadGLTF(full.string());
            if (!model.error.empty() || size_t(e.meshRef.index) >= model.meshes.size()) {
                /* log: "mesh '<path>' #<index> could not be loaded" */
            } else {
                comp::MeshComponent mc;
                mc.meshID = m_ctx.p_meshSystem->RegisterMesh(std::move(model.meshes[e.meshRef.index]));
                m_ctx.p_scene->InsertComponent(ent, mc);
                m_ctx.p_scene->InsertComponent(ent, comp::MeshRefComponent{e.meshRef.path, e.meshRef.index});
            }
        }

        if (!e.name.empty()) {
            comp::NameComponent nameComp;
            nameComp.name = e.name;
            m_ctx.p_scene->InsertComponent(ent, nameComp);
        }
    }


    for (size_t i = 0; i < entries.size(); ++i) {
        const Entry& e = entries[i];
        if (e.parent == NO_PARENT) continue;
        const Entity child = idToEntity.at(e.id);
        m_ctx.p_hierarchy->SetParent(child, idToEntity.at(e.parent));
        m_ctx.p_transformSystem->MarkDirty(child);
    }
    return  true;
}


bool SceneSerializer::Load(const std::filesystem::path& path, std::string& error) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs.is_open()) {
        error = "failed to open " + path.string();
        return false;
    }
    std::string text((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    std::vector<Entry> entries;
    if (!Parse(text, entries, error)) return false;
    return Apply(entries, error);
}

bool SceneSerializer::Save(const std::filesystem::path& path, std::string& error, int& skipped) const {
    skipped = 0;

    // pass 1: pick which entities are saved and give each a sequential file id
    std::unordered_map<Entity, int> entityToId;
    std::vector<Entity> saved;
    for (Entity e : m_ctx.p_scene->GetLivingEntities()) {
        entityToId.emplace(e, static_cast<int>(saved.size()));
        saved.push_back(e);
    }

    // pass 2: build the json
    json root;
    root["version"] = SUPPORTED_VERSION;
    json entities = json::array();
    for (Entity e : saved) {
        json item;
        item["id"] = entityToId.at(e);

        int parentId = NO_PARENT;
        uint32_t temp;
        if (m_ctx.p_hierarchy->HasParent(e)) {
            temp = m_ctx.p_hierarchy->TryGetParent(e).value();
            if (entityToId.contains(temp)) {
                parentId = entityToId.at(temp);
            }
        }
        // look up e's parent in Hierarchy; if it has one AND it's in entityToId,
        // parentId = entityToId.at(parent). Otherwise it stays -1.
        item["parent"] = parentId;
        const comp::TransformComponent& tComp = m_ctx.p_transformSystem->GetTransform(e);
        const comp::PrimitiveComponent* pComp = m_ctx.p_scene->TryGetComponent<comp::PrimitiveComponent>(e);
        const comp::NameComponent* nComp = m_ctx.p_scene->TryGetComponent<comp::NameComponent>(e);
        const comp::MaterialComponent* mComp = m_ctx.p_scene->TryGetComponent<comp::MaterialComponent>(e);
        item["transform"] = {{"pos",{tComp.position.x,tComp.position.y,tComp.position.z}},{"rot",{tComp.rotation.w,tComp.rotation.x,tComp.rotation.y,tComp.rotation.z}},{"scale",{tComp.scale.x,tComp.scale.y,tComp.scale.z}}};
        if (pComp) {
            item["primitive"] = {{"type",std::string(magic_enum::enum_name(pComp->type))},{"rings",pComp->rings},{"segments",pComp->segments},{"height",pComp->height},{"radius",pComp->radius}};
            item["HasMesh"] = true;
        }
        if (nComp) {
            item["name"] = nComp->name;
        }
        if (mComp) {
            const Material* mat = m_ctx.p_materialSystem->GetMaterial(mComp->materialID);
            if (mat) {
                json m;
                m["shader"] = mat->shaderKey.empty() ? "builtin:brdf" : mat->shaderKey;
                json props = json::object();
                for (const auto& p : mat->props)
                    std::visit([&](const auto& v) {
                        using T = std::decay_t<decltype(v)>;
                        if constexpr (std::is_same_v<T, float>) props[p.name] = v;
                        else if constexpr (std::is_same_v<T, TextureRef>) props[p.name] = v.path.empty() ? json(nullptr) : json(v.path);
                        else                                    props[p.name] = {v.x, v.y, v.z};
                    }, p.value);
                m["props"] = std::move(props);
                item["material"] = std::move(m);
            }
        }

        if (const comp::LightComponent* lComp = m_ctx.p_scene->TryGetComponent<comp::LightComponent>(e)) {
            item["light"] = {
                {"type",      std::string(magic_enum::enum_name(lComp->type))},
                {"color",     {lComp->color.x, lComp->color.y, lComp->color.z}},
                {"intensity", lComp->intensity},
                {"range",     lComp->range},
                {"spotSize",  lComp->spotSize},
                {"blend",     lComp->blend}
            };
        }
        if (const auto* ref = m_ctx.p_scene->TryGetComponent<comp::MeshRefComponent>(e))
            item["meshRef"] = {{"path", ref->path}, {"index", ref->index}};

        entities.push_back(std::move(item));
    }
    root["entities"] = std::move(entities);

    const std::string text = root.dump(2);

    std::filesystem::path tmp = path;
    tmp += ".tmp";
    {
        std::ofstream ofs(tmp, std::ios::binary | std::ios::trunc);
        if (!ofs) { error = "failed to open " + tmp.string(); return false; }
        ofs << text;
        ofs.flush();
        if (!ofs.good()) {
            ofs.close();
            std::error_code rm;
            std::filesystem::remove(tmp, rm);
            error = "failed to write " + tmp.string();
            return false;
        }
    }   // stream closes here, before the rename

    std::error_code ec;
    std::filesystem::rename(tmp, path, ec);
    if (ec) {
        std::filesystem::remove(tmp, ec);
        error = "failed to replace " + path.string();
        return false;
    }
    return true;
}


