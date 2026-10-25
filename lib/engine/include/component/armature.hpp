#pragma once
#include "mathpp.hpp"
struct Transform {
    mathpp::vec3f position = {0.0f,0.0f,0.0f};
    mathpp::quatf rotation = mathpp::quatf();
    mathpp::vec3f scale = {1.0f,1.0f,1.0f};
};

struct Armature {
    std::vector<std::string> boneNames;
    std::vector<int> parentIndices;              // -1 for root
    std::vector<mathpp::mat4f> inverseBindPoses; // one per bone, same order as boneNames
    std::vector<Transform> restTransforms; // each bone's local transform at bind time, read from the file
};

namespace comp {
    struct ArmatureComponent {
        Armature armature;
        std::vector<mathpp::mat4f> currentLocalPose; //for st
    };
}