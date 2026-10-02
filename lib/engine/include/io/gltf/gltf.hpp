#pragma once
#include <vector>
#include <optional>
#include <string>
#include "core/system/armature/armature.hpp"
#include "render/mesh/mesh.hpp"


struct GLTFModel {
    std::vector<Mesh> meshes;
    std::optional<Armature> armature;
};


GLTFModel LoadGLTF(const std::string& path);