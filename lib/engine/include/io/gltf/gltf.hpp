#pragma once
#include <vector>
#include <optional>
#include <string>
#include <mathpp.hpp>
#include "core/system/armature/armature.hpp"
#include "render/mesh/mesh.hpp"

struct GLTFImage {
    std::string extension;                 // ".png" or ".jpg"
    std::vector<unsigned char> bytes;      // original encoded file, untouched
};

struct GLTFMaterial {
    bool  hasPbr = false;
    int   baseColorImage = -1;
    float baseColorFactor[4] = {1, 1, 1, 1};
    float metallic  = 0.0f;
    float roughness = 1.0f;
    int   emissiveImage = -1;
    float emissiveFactor[3] = {0, 0, 0};
    int uvSet = 0;
};

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
    std::vector<int>          meshMaterial;
    std::vector<GLTFMaterial> materials;
    std::vector<GLTFImage>    images;
    std::vector<GLTFNode> nodes;
    std::optional<Armature> armature;
    std::string error;                     // non-empty = load failed
    std::string warning;                   // non-fatal, show in the UI
};

GLTFModel LoadGLTF(const std::string& path);