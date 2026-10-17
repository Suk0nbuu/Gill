#pragma once
#include "core/system/asset/assetID.hpp"
#include <string>
#include <variant>
#include <vector>
#include "mathpp.hpp"

enum class PropType { Float, Color3 };                  // Texture later
using PropValue = std::variant<float, mathpp::vec3f>;

struct PropertyDecl { std::string name; PropType type; PropValue def; float min = 0, max = 1; };
struct ShaderDef {
    std::string vert, frag; std::vector<PropertyDecl> props;

    const PropertyDecl* Find(const std::string& n) const {
        for (const auto& p : props) if (p.name == n) return &p;
        return nullptr;
    }
};

struct MaterialProperty { std::string name; PropValue value; };
struct Material {
    std::string shaderKey;            // source of truth, serialized
    AssetHandle shaderID;             // runtime cache, set together with shaderKey
    AssetHandle textureID;            // keep until textures become a property (Textured viewport mode uses it)
    std::vector<MaterialProperty> props;
};

namespace comp {



    struct MaterialComponent {
        AssetHandle materialID;
    };
}