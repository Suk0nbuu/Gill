#pragma once
#include "core/system/asset/asset.hpp"
#include "component/material.hpp"
#include "render/shader/shader.hpp"
#include "render/texture/texture.hpp"
#include <string>
#include <filesystem>
#include <unordered_map>
#include "render/shader/genShaderCache.hpp"



class ProjectPaths;

class MaterialSystem {
public:
    MaterialSystem(ProjectPaths* projectPaths);
    [[nodiscard]]const Material* GetMaterial(AssetHandle handle) const;
    [[nodiscard]] Material* GetMaterial(AssetHandle handle);
    [[nodiscard]] AssetHandle GetShaderByKey(const std::string& key);
    [[nodiscard]]bool HasMaterial(AssetHandle handle) const;
    const ShaderDef* GetShaderDef(const std::string& key) const;
    AssetHandle RegisterMaterial(Material&& material);
    void RemoveMaterial(AssetHandle handle);

    Material MakeMaterial(const std::string& key);
    [[nodiscard]]const Shader* GetShader(AssetHandle handle) const;
    [[nodiscard]]bool HasShader(AssetHandle handle) const;
    AssetHandle LoadShaderFile(const std::string& vertPath, const std::string& fragPath);
    void RemoveShader(AssetHandle handle);

    std::vector<std::string> ShaderKeys() const;
    void SetShader(Material& m, const std::string& key);


    [[nodiscard]]const Texture* GetTexture(AssetHandle handle) const;
    [[nodiscard]]bool HasTexture(AssetHandle handle) const;
    AssetHandle LoadTextureFile(const std::string& path);
    void RemoveTexture(AssetHandle handle);
    void ClearGraph(Material& m);
    bool CreateDefaultGraph(Material& m, std::string& err);


    AssetHandle GetOrLoadTexture(const std::string& relPath);
    bool CompileGraph(Material& m, std::string& err);
    const CodegenResult& GetGeneratedProgram(int id) const { return m_generatedShaders.Program(id); }



private:
    AssetManager<Material> m_materialManager;
    AssetManager<Shader> m_shaderManager;
    AssetManager<Texture> m_textureManager;
    std::unordered_map<std::string, AssetHandle> m_textureByPath;




    std::unordered_map<std::string, ShaderDef>    m_defs;      // key -> definition
    std::unordered_map<std::string, AssetHandle>  m_handles;   // key -> loaded shader (lazy)
    ProjectPaths* p_projectPath = nullptr;
    GeneratedShaderCache m_generatedShaders;


};
