#pragma once
#include "render/shader/shader.hpp"
#include <cstdint>
#include <unordered_map>
#include <memory>
enum ShaderFeature : uint32_t {
    Feature_None     = 0,
    Feature_Skinning = 1 << 0,
};

class ShaderVariantCache {
public:
    Shader* Get(const std::string& vertPath, const std::string& fragPath, uint32_t features);
private:
    struct Key {
        std::string vertPath, fragPath;
        uint32_t features;
        bool operator==(const Key& other) const {
            return vertPath == other.vertPath && fragPath == other.fragPath && features == other.features;
        }
    };
    struct KeyHash {
        size_t operator()(const Key& k) const {
            return std::hash<std::string>()(k.vertPath) ^ std::hash<std::string>()(k.fragPath) ^ std::hash<uint32_t>()(k.features);
        }
    };
    std::unordered_map<Key, std::unique_ptr<Shader>, KeyHash> m_cache;
};