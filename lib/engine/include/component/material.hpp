#pragma once
#include "core/system/asset/assetID.hpp"
#include "component/textureRef.hpp"
#include "shadergraph/graph.hpp"
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include "mathpp.hpp"

enum class PropType { Float, Color3, Texture };
using PropValue = std::variant<float, mathpp::vec3f, TextureRef>;

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
    std::string shaderKey;            // builtin shader key, serialized; ignored while `graph` is set
    AssetHandle shaderID;             // runtime cache, set together with shaderKey
    std::vector<MaterialProperty> props;
    std::optional<ShaderGraph> graph; // when set, the material renders from this graph, serialized
    int graphShaderId = -1;           // runtime cache from MaterialSystem::CompileGraph, never serialized
};

namespace comp {
    struct MaterialComponent {
        AssetHandle materialID;
    };
}