#pragma once
#include "mat4.hpp"
#include <cstdint>
#include <memory>
#include "core/system/asset/asset.hpp"
#include "render/shader/shader.hpp"
#include "render/texture/texture.hpp"
#include "component/material.hpp"
#include "component/mesh.hpp"
#include "core/system/mesh/mesh.hpp"
#include "render/shader/shaderVariant.hpp"

class ArmatureSystem;
class TransformSystem;
class Scene;
class MeshSystem;
class MaterialSystem;


using Entity = uint32_t;




enum class ViewportMode : int {
    Solid = 1,
    Textured = 2,
    Rendered = 3
};

class Renderer {
    public:
    ~Renderer();
    void Init(TransformSystem* transformSystem, MeshSystem* meshSystem,MaterialSystem* materialSystem,ArmatureSystem* armatureSystem);
    void renderScene(const Scene* scene,const mathpp::mat4f& view,const mathpp::mat4f& projection,const mathpp::vec3f& viewVec);
    void SetViewportMode(ViewportMode mode);
    ViewportMode GetViewportMode();
private:
    void UploadSkinningPalette(const Scene* scene, Entity entity, const Shader& shader);
    void DrawEntity(const Scene* scene, Entity entity, const mathpp::mat4f& view,const comp::MeshComponent& meshComp, const mathpp::mat4f& proj,const mathpp::vec3f& viewVec);
    std::unique_ptr<Shader> solidShader;
    std::unique_ptr<Shader> textureShader;
    std::unique_ptr<Texture> matCapTexture;
    std::unique_ptr<Texture> fallBackTexture;

    ShaderVariantCache m_shaderCache;

    TransformSystem* p_transformSystem;
    MeshSystem* p_meshSystem;
    MaterialSystem* p_materialSystem;
    ArmatureSystem* p_armatureSystem;
    ViewportMode em_viewportMode = ViewportMode::Solid;
};