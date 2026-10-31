#pragma once
#include <memory>
#include <memory>


class Shader;
struct Armature;
struct Transform;

class ArmaturePicker {
public:
    void Init();
    void Draw(const Armature& armature);

private:
    void DrawBone(Transform* transform);
    void DrawBoneID(Transform* transform);
    std::unique_ptr<Shader> m_boneShader;
    std::unique_ptr<Shader>m_pickShader;
    unsigned int m_pickFBO, m_pickTexture, m_pickDepthRenderBuffer;
    unsigned int m_pickWidth = 0, m_pickHeight = 0;
};