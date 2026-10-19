#pragma once
#include <vector>
#include <optional>
#include <string>
#include <mathpp.hpp>
#include "core/system/armature/armature.hpp"
#include "render/mesh/mesh.hpp"

struct GLTFNode {
    std::string name;
    int parent = -1;                       // index into GLTFModel::nodes, -1 = root
    mathpp::vec3f pos{0.0f, 0.0f, 0.0f};
    mathpp::quatf rot;                     // set explicitly to identity below
    mathpp::vec3f scale{1.0f, 1.0f, 1.0f};
    std::vector<int> meshes;               // indices into GLTFModel::meshes (one per triangle primitive)
    int skin = -1;                         // glTF skin index, -1 = not skinned
    bool isJoint = false;                  // true for armature joints: don't make entities for these
};

struct GLTFModel {
    std::vector<Mesh> meshes;
    std::vector<GLTFNode> nodes;
    std::optional<Armature> armature;
    std::string error;                     // non-empty = load failed
    std::string warning;                   // non-fatal, show in the UI
};

GLTFModel LoadGLTF(const std::string& path);