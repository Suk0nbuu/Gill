#include "core/system/armature/armature.hpp"

Armature BuildArmatureFromSkin(const cgltf_skin* skin) {
    Armature armature;
    size_t boneCount = skin->joints_count;

    armature.boneNames.resize(boneCount);
    armature.parentIndices.resize(boneCount);
    armature.inverseBindPoses.resize(boneCount);
    armature.bindLocalPose.resize(boneCount);

    for (size_t i = 0; i < boneCount; ++i) {
        cgltf_node* joint = skin->joints[i];
        armature.boneNames[i] = joint->name ? joint->name : ("bone_" + std::to_string(i));

        int parentIndex = -1;
        if (joint->parent) {
            for (size_t j = 0; j < boneCount; ++j) {
                if (skin->joints[j] == joint->parent) {
                    parentIndex = static_cast<int>(j);
                    break;
                }
            }
        }
        armature.parentIndices[i] = parentIndex;

        float localM[16];
        cgltf_node_transform_local(joint, localM);
        armature.bindLocalPose[i] = mathpp::MakeMat4FromColumnMajorElements<float>(localM);
    }

    if (skin->inverse_bind_matrices) {
        for (size_t i = 0; i < boneCount; ++i) {
            float m[16];
            cgltf_accessor_read_float(skin->inverse_bind_matrices, i, m, 16);
            armature.inverseBindPoses[i] = mathpp::MakeMat4FromColumnMajorElements<float>(m);
        }
    } else {
        for (size_t i = 0; i < boneCount; ++i) {
            armature.inverseBindPoses[i] = mathpp::mat4f();
        }
    }

    return armature;
}

std::vector<mathpp::mat4f> ArmatureSystem::ComputeWorldTransforms(const comp::ArmatureComponent& armComp) const {
    const Armature& arm = armComp.armature;
    std::vector<mathpp::mat4f> world(arm.boneNames.size());
    for (size_t i = 0; i < arm.boneNames.size(); ++i) {
        int parent = arm.parentIndices[i];
        world[i] = (parent == -1) ? armComp.currentLocalPose[i] : world[parent] * armComp.currentLocalPose[i];
    }
    return world;
}

std::vector<mathpp::mat4f> ArmatureSystem::ComputeSkinningPalette(const comp::ArmatureComponent& armComp) const {
    std::vector<mathpp::mat4f> world = ComputeWorldTransforms(armComp);
    std::vector<mathpp::mat4f> palette(world.size());
    for (size_t i = 0; i < world.size(); ++i) {
        palette[i] = world[i] * armComp.armature.inverseBindPoses[i];
    }
    return palette;
}

void ArmatureSystem::ResetToBindPose(comp::ArmatureComponent& armComp) const {
    armComp.currentLocalPose = armComp.armature.bindLocalPose;
}

ArmatureSystem::~ArmatureSystem() = default;
