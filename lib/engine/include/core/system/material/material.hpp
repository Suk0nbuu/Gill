#pragma once
#include "core/system/asset/asset.hpp"
#include "component/material.hpp"
#include "render/shader/shader.hpp"
#include "render/texture/texture.hpp"
#include <string>

class MaterialSystem {
public:

    [[nodiscard]]const Material* GetMaterial(AssetHandle handle) const;
    [[nodiscard]] Material* GetMaterial(AssetHandle handle);
    [[nodiscard]] AssetHandle GetDefaultShader();
    [[nodiscard]]bool HasMaterial(AssetHandle handle) const;
    AssetHandle RegisterMaterial(Material&& material);
    void RemoveMaterial(AssetHandle handle);


    [[nodiscard]]const Shader* GetShader(AssetHandle handle) const;
    [[nodiscard]]bool HasShader(AssetHandle handle) const;
    AssetHandle LoadShaderFile(const std::string& vertPath, const std::string& fragPath);
    void RemoveShader(AssetHandle handle);


    [[nodiscard]]const Texture* GetTexture(AssetHandle handle) const;
    [[nodiscard]]bool HasTexture(AssetHandle handle) const;
    AssetHandle LoadTextureFile(const std::string& path);
    void RemoveTexture(AssetHandle handle);

private:
    AssetManager<Material> m_materialManager;
    AssetManager<Shader> m_shaderManager;
    AssetManager<Texture> m_textureManager;
    AssetHandle m_defaultBRDF;

};