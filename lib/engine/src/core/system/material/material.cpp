#include "core/system/material/material.hpp"
#include <algorithm>
#include "core/project/projectPath.hpp"
#include "shaderGraph/nodeRegistry.hpp"

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


MaterialSystem::MaterialSystem(ProjectPaths* projectPaths) {
    p_projectPath = projectPaths;
    m_defs["builtin:brdf"] = { "asset/shader/core/surfaceVert.glsl", "asset/shader/pbr/brdf/brdfFrag.glsl", {
        {"albedo",    PropType::Color3, mathpp::vec3f{0.8f,0.8f,0.8f}},
        {"metallic",  PropType::Float,  0.2f, 0.0f,  1.0f},
        {"roughness", PropType::Float,  1.0f, 0.05f, 1.0f},
        {"albedoTex", PropType::Texture, TextureRef{}},
    }};
    m_defs["builtin:emission"] = {"asset/shader/core/surfaceVert.glsl","asset/shader/pbr/emission/emissionFrag.glsl",{
    {"color",PropType::Color3,mathpp::vec3f{1.0f,1.0f,1.0f}},
        {"strength",PropType::Float,1.0f,0.0f,20.0f},

    }};
    m_defs["builtin:toon"] = { "asset/shader/core/surfaceVert.glsl", "asset/shader/npr/toon/toonFrag.glsl", {
        {"albedo",          PropType::Color3,  mathpp::vec3f{1.0f, 1.0f, 1.0f}},
        {"albedoTex",       PropType::Texture, TextureRef{}},
        {"shadowColor",     PropType::Color3,  mathpp::vec3f{0.55f, 0.55f, 0.7f}},
        {"shadowThreshold", PropType::Float,   0.5f,  0.0f, 1.0f},
        {"shadowSoftness",  PropType::Float,   0.02f, 0.0f, 0.5f},
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
    const std::filesystem::path full = p_projectPath->Resolve(rel);
    if (!std::filesystem::is_regular_file(full)) return AssetHandle{};
    AssetHandle h = LoadTextureFile(full.string());
    m_textureByPath.emplace(rel, h);
    return h;
}

bool MaterialSystem::CompileGraph(Material& m, std::string& err) {
    m.graphShaderId = -1;
    if (!m.graph) { err = "material has no graph"; return false; }

    for (auto& n : m.graph->nodes) {                      // re-resolve texture handles from their saved paths
        const NodeDef* def = NodeRegistry::Get().Find(n.type);
        if (def && def->usesImage && !n.image.path.empty())
            n.image.handle = GetOrLoadTexture(n.image.path);
    }

    CodegenResult r = GenerateFragment(*m.graph, NodeRegistry::Get());
    if (!r.ok) { err = r.error; return false; }
    m.graphShaderId = m_generatedShaders.Register(r);
    return true;
}

void MaterialSystem::ClearGraph(Material& m) {
    m.graph.reset();
    m.graphShaderId = -1;
}

bool MaterialSystem::CreateDefaultGraph(Material& m, std::string& err) {
    ShaderGraph g = MakeToonGraph();
    for (const auto& p : m.props)                         // keep a texture the material already had
        if (p.name == "albedoTex")
            if (const auto* t = std::get_if<TextureRef>(&p.value))
                for (auto& n : g.nodes) if (n.type == "texture.image") n.image = *t;
    m.graph = std::move(g);
    return CompileGraph(m, err);
}