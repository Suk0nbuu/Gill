#include "render/shader/shaderVariant.hpp"
#include "render/shader/shader.hpp"
#include "render/data/bindings.hpp"

Shader* ShaderVariantCache::Get(const std::string& vertPath, const std::string& fragPath, uint32_t features) {
    Key key{vertPath, fragPath, features};
    auto it = m_cache.find(key);
    if (it != m_cache.end()) return it->second.get();

    std::string defines;
    if (features & Feature_Skinning) {
        defines += "#define USE_SKINNING\n";
        defines += "#define MAX_BONES " + std::to_string(kMaxBones) + "\n";
    }


    auto shader = std::make_unique<Shader>(vertPath, fragPath, defines);
    Shader* raw = shader.get();
    m_cache[key] = std::move(shader);
    return raw;
}

namespace {
    std::string BuildDefines(uint32_t features) {
        std::string d;
        if (features & Feature_Skinning) {
            d += "#define USE_SKINNING\n";
            d += "#define MAX_BONES " + std::to_string(kMaxBones) + "\n";
        }
        return d;
    }
}



Shader* ShaderVariantCache::GetGenerated(const std::string& vertPath, int genId,const std::string& fragSource, uint32_t features) {
    Key key{vertPath, "generated:" + std::to_string(genId), features};   // memakai ulang field fragPath
    auto it = m_cache.find(key);
    if (it != m_cache.end()) return it->second.get();

    auto shader = std::make_unique<Shader>(
        Shader::FromGeneratedFragment(vertPath, fragSource, "asset/shader/core", BuildDefines(features)));
    Shader* raw = shader.get();
    m_cache[key] = std::move(shader);          // shader yang gagal juga di-cache, supaya error tidak berulang tiap draw
    return raw;
}