#include "core/system/armature/armature.hpp"

#include <ranges>

namespace {
    std::string ResolveNodeName(const cgltf_node* node, const cgltf_data* data) {
        if (!node->name || node->name[0] == '\0') {
            size_t index = node - data->nodes;
            return "node_" + std::to_string(index);
        }
        return node->name;
    }
}

Armature BuildArmatureFromSkin(const cgltf_skin* skin,const cgltf_data* data) {
    Armature armature;
    size_t boneCount = skin->joints_count;

    armature.boneNames.resize(boneCount);
    armature.parentIndices.resize(boneCount);
    armature.inverseBindPoses.resize(boneCount);
    armature.restTransforms.resize(boneCount);

    for (size_t i = 0; i < boneCount; ++i) {
        cgltf_node* joint = skin->joints[i];
        armature.boneNames[i] = ResolveNodeName(joint,data);
        Transform transform;


        cgltf_float pos [3], rot[4], scale[3];
        if (joint->has_translation) {
            pos[0] = joint->translation[0];
            pos[1] = joint->translation[1];
            pos[2] = joint->translation[2];
             transform.position = {pos[0], pos[1], pos[2]};
        }
        else if (joint->has_rotation) {
            rot[0] = joint->rotation[3]; //w
            rot[1] = joint->rotation[3]; //x
            rot[2] = joint->rotation[3];// y
            rot[3] = joint->rotation[3];// z
            transform.rotation = {rot[0], rot[1], rot[2], rot[3]};
        }
        else if (joint->has_scale) {
            scale[0] = joint->scale[0];
            scale[1] = joint->scale[1];
            scale[2] = joint->scale[2];
            transform.scale = {scale[0], scale[1], scale[2]};
        }
        else if (joint->has_matrix) {
            mathpp::mat4f mat = mathpp::MakeMat4FromColumnMajorElements<float>(joint->matrix);
            transform.position = mathpp::TranslateFromMat4(mat);
            transform.scale = mathpp::ScaleFromMat4(mat);
            transform.rotation  = mathpp::QuatFromMat4(mat);
        }

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
        armature.restTransforms[i] = transform;
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
    armComp.currentLocalPose = armComp.armature.restTransforms;
}

ArmatureSystem::~ArmatureSystem() = default;
