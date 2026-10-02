#pragma once
struct Armature {
    std::vector<std::string> boneNames;
    std::vector<int> parentIndices;              // -1 for root
    std::vector<mathpp::mat4f> inverseBindPoses; // one per bone, same order as boneNames
    std::vector<mathpp::mat4f> bindLocalPose; // each bone's local transform at bind time, read from the file
};

namespace comp {
    struct ArmatureComponent {
        Armature armature;
        std::vector<mathpp::mat4f> currentLocalPose;
    };
}