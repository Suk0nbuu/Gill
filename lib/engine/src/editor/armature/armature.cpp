#include "editor/armature/armature.hpp"
#include "glad/gl.h"
#include "render/shader/shader.hpp"

void ArmaturePicker::Init() {
    m_boneShader = std::make_unique<Shader>("","");
    m_pickShader = std::make_unique<Shader>("","");
}
