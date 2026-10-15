#include "core/system/material/material.hpp"

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

AssetHandle MaterialSystem::GetDefaultShader()  {
    if (m_defaultBRDF.index == UINT32_MAX) {
        m_defaultBRDF = LoadShaderFile("asset/shader/pbr/brdf/brdfVert.glsl", "asset/shader/pbr/brdf/brdfFrag.glsl");
    }
   return m_defaultBRDF;
}

AssetHandle MaterialSystem::GetShaderByKey(const std::string &key) {
    if (m_shaders.contains(key)) {
        return m_shaders[key];
    }
}

Material* MaterialSystem::GetMaterial(AssetHandle id) {
    if (m_materialManager.Has(id)) {
        return m_materialManager.Get(id);
    }
    return nullptr;
}