#pragma once
#include "core/system/asset/assetID.hpp"
#include "mathpp.hpp"

struct Material {
    AssetHandle shaderID;
    AssetHandle textureID;
    mathpp::vec3f albedo{0.8f,0.8f,0.8f};
    float metallic{0.2f};
    float roughness{1.0f};

};

namespace comp {



    struct MaterialComponent {
        AssetHandle materialID;
    };
}