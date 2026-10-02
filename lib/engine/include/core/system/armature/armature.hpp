#pragma once
#include <vector>
#include "mathpp.hpp"
#include <string>
#include "component/armature.hpp"
#include "cgltf.h"

Armature BuildArmatureFromSkin(const cgltf_skin* skin);


class ArmatureSystem {
public:
    ~ArmatureSystem();
    std::vector<mathpp::mat4f> ComputeWorldTransforms(const comp::ArmatureComponent& armComp) const;
    std::vector<mathpp::mat4f> ComputeSkinningPalette(const comp::ArmatureComponent& armComp) const;
    void ResetToBindPose(comp::ArmatureComponent& armComp) const;
};