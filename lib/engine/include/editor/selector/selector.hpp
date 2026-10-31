#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <optional>
#include "mathpp.hpp"
#include <cstdint>
#include "component/mesh.hpp"
#include "editor/picker/picker.hpp"

using Entity = uint32_t;

class Shader;
class Scene;
class TransformSystem;
class MeshSystem;

class Selector {
    public:
    Selector() = default;
    void Init();
    void RenderScene(const Scene* scene,TransformSystem* transformSystem,MeshSystem* meshSystem);
    std::optional<Entity> ReadEntityAt(int mx, int my);

private:
    void RenderEntityID(const Scene* scene,Entity entity, const comp::MeshComponent& meshComp,TransformSystem* transformSystem,MeshSystem* meshSystem);
    unsigned int m_width = 0, m_height = 0;
    std::unique_ptr<Shader> up_selectShader;
    gill_editor::Picker* p_picker;
};
