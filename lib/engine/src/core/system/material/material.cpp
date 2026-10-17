#include "core/system/material/material.hpp"
#include <algorithm>

const Material *MaterialSystem::GetMaterial(AssetHandle handle) const {
    if (m_materialManager.Has(handle)) {
        return m_materialManager.Get(handle);
    }
    return nullptr;
}

bool MaterialSystem::HasMaterial(AssetHandle handle) const {
    return m_materialManager.Has(handle);
}

AssetHandle MaterialSystem::RegisterMaterial(Material &&material) {
    return m_materialManager.Load(material);
}

const Shader *MaterialSystem::GetShader(AssetHandle handle) const {
    if (m_shaderManager.Has(handle)) {
        return m_shaderManager.Get(handle);
    }
    return nullptr;
}


const Texture *MaterialSystem::GetTexture(AssetHandle handle) const {
    if (m_textureManager.Has(handle)) {
        return m_textureManager.Get(handle);
    }
    return nullptr;
}

bool MaterialSystem::HasShader(AssetHandle handle) const {
    return m_shaderManager.Has(handle);
}

bool MaterialSystem::HasTexture(AssetHandle handle) const {
    return m_textureManager.Has(handle);
}

void MaterialSystem::RemoveMaterial(AssetHandle handle) {
    m_materialManager.Unload(handle);
}

void MaterialSystem::RemoveTexture(AssetHandle handle) {
    m_textureManager.Unload(handle);
}

void MaterialSystem::RemoveShader(AssetHandle handle) {
    m_shaderManager.Unload(handle);
}

AssetHandle MaterialSystem::LoadTextureFile(const std::string &path) {
    return m_textureManager.Load(Texture(path));
}

AssetHandle MaterialSystem::LoadShaderFile(const std::string &vertPath, const std::string &fragPath) {
    return m_shaderManager.Load(Shader(vertPath, fragPath));
}



AssetHandle MaterialSystem::GetShaderByKey(const std::string& key) {
    if (auto it = m_handles.find(key); it != m_handles.end()) return it->second;
    auto d = m_defs.find(key);
    if (d == m_defs.end()) return AssetHandle{};            // unknown key: invalid, never fall off the end
    AssetHandle h = LoadShaderFile(d->second.vert, d->second.frag);
    m_handles.emplace(key, h);
    return h;
}

Material* MaterialSystem::GetMaterial(AssetHandle id) {
    if (m_materialManager.Has(id)) {
        return m_materialManager.Get(id);
    }
    return nullptr;
}

Material MaterialSystem::MakeMaterial(const std::string& key) {
    Material m; m.shaderKey = key; m.shaderID = GetShaderByKey(key);
    if (auto d = m_defs.find(key); d != m_defs.end())
        for (const auto& decl : d->second.props) m.props.push_back({decl.name, decl.def});
    return m;
}


MaterialSystem::MaterialSystem() {
    m_defs["builtin:brdf"] = { "asset/shader/pbr/brdf/brdfVert.glsl", "asset/shader/pbr/brdf/brdfFrag.glsl", {
        {"albedo",    PropType::Color3, mathpp::vec3f{0.8f,0.8f,0.8f}},
        {"metallic",  PropType::Float,  0.2f, 0.0f,  1.0f},
        {"roughness", PropType::Float,  1.0f, 0.05f, 1.0f},
        {"albedoTex", PropType::Texture, TextureRef{}},
    }};
    m_defs["builtin:emission"] = {"asset/shader/pbr/brdf/brdfVert.glsl","asset/shader/pbr/emission/emissionFrag.glsl",{
    {"color",PropType::Color3,mathpp::vec3f{1.0f,1.0f,1.0f}},
        {"strength",PropType::Float,1.0f,0.0f,20.0f},

    }};
}

const ShaderDef *MaterialSystem::GetShaderDef(const std::string &key) const {
    auto it = m_defs.find(key);
    return it == m_defs.end() ? nullptr : &it->second;
}

std::vector<std::string> MaterialSystem::ShaderKeys() const {
    std::vector<std::string> k; for (auto& [key, _] : m_defs) k.push_back(key);
    std::sort(k.begin(), k.end()); return k;
}

void MaterialSystem::SetShader(Material &m, const std::string &key) {
    if (!GetShaderDef(key)) return;
    Material fresh = MakeMaterial(key);
    for (auto& np : fresh.props)
        for (const auto& op : m.props)
            if (op.name == np.name && op.value.index() == np.value.index()) { np.value = op.value; break; }
    m.shaderKey = key; m.shaderID = fresh.shaderID; m.props = std::move(fresh.props);
}

AssetHandle MaterialSystem::GetOrLoadTexture(const std::string& rel) {
    if (auto it = m_textureByPath.find(rel); it != m_textureByPath.end()) return it->second;
    const std::filesystem::path full = m_projectRoot / rel;
    if (!std::filesystem::is_regular_file(full)) return AssetHandle{};   // don't cache failures
    AssetHandle h = LoadTextureFile(full.string());
    m_textureByPath.emplace(rel, h);
    return h;
}