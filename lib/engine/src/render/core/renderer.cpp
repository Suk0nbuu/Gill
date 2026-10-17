#include "render/core/renderer.hpp"
#include "render/shader/shader.hpp"
#include "core/system/light/light.hpp"
#include "render/texture/texture.hpp"
#include "core/system/transform/transform.hpp"
#include "core/system/mesh/mesh.hpp"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "core/system/armature/armature.hpp"
#include "component/armature.hpp"
#include "core/system/material/material.hpp"
#include "scene/scene.hpp"
#include "render/data/bindings.hpp"
#include "render/data/cameraBlock.hpp"
#include "render/data/lightBlock.hpp"
#include "render/data/uniform.hpp"
#include <type_traits>


void Renderer::Init(Scene* scene,TransformSystem* transformSystem,MeshSystem* meshSystem,MaterialSystem* materialSystem,ArmatureSystem* armatureSystem) {
    glEnable(GL_DEPTH_TEST);
    p_scene = scene;
    p_transformSystem = transformSystem;
    p_meshSystem = meshSystem;
    p_materialSystem = materialSystem;
    p_armatureSystem = armatureSystem;
    up_lightSystem = std::make_unique<LightSystem>();
    up_lightSystem->Init(transformSystem);
    textureShader = std::make_unique<Shader>("asset/shader/textureShader/textureVert.glsl","asset/shader/textureShader/textureFrag.glsl");
    solidShader = std::make_unique<Shader>("asset/shader/solidShader/solidVert.glsl","asset/shader/solidShader/solidFrag.glsl");
    matCapTexture = std::make_unique<Texture>("asset/texture/core/SolidTex2.png");
    fallBackTexture = std::make_unique<Texture>("asset/texture/core/Debugempty.png");
    up_cameraUBO = std::make_unique<UniformBuffer>(sizeof(CameraBlock), kCameraBinding);
    up_lightUBO = std::make_unique<UniformBuffer>(sizeof(LightBlock), kLightBinding);
}

void Renderer::SetViewportMode(ViewportMode mode) {
    em_viewportMode = mode;
}

ViewportMode Renderer::GetViewportMode() {
    return em_viewportMode;
}

void Renderer::renderScene(const Scene* scene, const mathpp::mat4f& view, const mathpp::mat4f& projection,const mathpp::vec3f& viewVec) {
    UploadCamera(view,projection,viewVec);
    UploadLight();
    scene->ForEach<comp::MeshComponent>([this, scene, &view, &projection,&viewVec](Entity entity,const comp::MeshComponent& meshComp) {
        DrawEntity(scene, entity, view, meshComp,projection,viewVec);
    });
}






void Renderer::DrawEntity(const Scene* scene, Entity entity,
                           const mathpp::mat4f& view,const comp::MeshComponent& meshComp, const mathpp::mat4f& proj, const mathpp::vec3f& viewVec) {
    const comp::MaterialComponent* matComp = scene->TryGetComponent<comp::MaterialComponent>(entity);
    const Material* mat = matComp ? p_materialSystem->GetMaterial(matComp->materialID) : nullptr;
    auto mesh = p_meshSystem->GetMesh(meshComp.meshID);
    if (!mesh) return;

    uint32_t features = mesh->IsSkinned() ? Feature_Skinning : Feature_None;

    if (em_viewportMode == ViewportMode::Solid) {
        Shader* shader = m_shaderCache.Get("asset/shader/solidShader/solidVert.glsl", "asset/shader/solidShader/solidFrag.glsl", features);
        shader->Use();
        mathpp::mat4f model = p_transformSystem->GetWorldTransform(entity);
        shader->setMat4f("model", model);
        mathpp::mat3f normalMat = mathpp::normal_matrix(view * model);
        shader->setMat3f("normalMatrix", normalMat);
        matCapTexture->Bind(0);
        shader->setInt("matCap", 0);

        if (features & Feature_Skinning) UploadSkinningPalette(scene, entity, *shader);
    } else if (em_viewportMode == ViewportMode::Rendered) {
        const Shader* baseShader = mat ? p_materialSystem->GetShader(mat->shaderID) : nullptr;
        const Shader* shader = baseShader ? baseShader : solidShader.get();
        shader->Use();
        if (mat) UploadMaterial(*mat, *shader);
        shader->setMat4f("model", p_transformSystem->GetWorldTransform(entity));
    }
    else {
        Shader* shader = m_shaderCache.Get("asset/shader/textureShader/textureVert.glsl", "asset/shader/textureShader/textureFrag.glsl", features);
        shader->Use();
        const Texture* tex = mat ? FindMaterialTexture(*mat) : nullptr;
        if (!tex) tex = fallBackTexture.get();
        tex->Bind(0);
        shader->setInt("meshTexture", 0);
        shader->setMat4f("model", p_transformSystem->GetWorldTransform(entity));

        if (features & Feature_Skinning) UploadSkinningPalette(scene, entity, *shader);
    }

    mesh->Draw();
}

Renderer::~Renderer() = default;

void Renderer::UploadSkinningPalette(const Scene* scene, Entity entity, const Shader& shader) {
    const auto* armComp = scene->TryGetComponent<comp::ArmatureComponent>(entity);
    if (!armComp) return;
    std::vector<mathpp::mat4f> palette = p_armatureSystem->ComputeSkinningPalette(*armComp);
    for (size_t i = 0; i < palette.size(); ++i) {
        shader.setMat4f("boneMatrices[" + std::to_string(i) + "]", palette[i]);
    }
}

void Renderer::UploadCamera(const mathpp::mat4f& view, const mathpp::mat4f& proj, const mathpp::vec3f& camPos) {
    CameraBlock cb{};
    cb.view = view;
    cb.projection = proj;
    cb.cameraPos[0] = camPos.x;
    cb.cameraPos[1] = camPos.y;
    cb.cameraPos[2] = camPos.z;
    cb.cameraPos[3] = 1.0f;
    up_cameraUBO->Update(&cb, sizeof(cb));
}

void Renderer::UploadLight() {
    LightBlock lb{};
    up_lightSystem->Gather(p_scene,lb);
    up_lightUBO->Update(&lb, sizeof(lb));
}

void Renderer::UploadMaterial(const Material& m, const Shader& s) const {
    int slot = 0;
    for (const auto& p : m.props) {
        std::visit([&](const auto& v) {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, float>)              s.setFloat(p.name, v);
            else if constexpr (std::is_same_v<T, mathpp::vec3f>) s.setVec3f(p.name, v);
            else if constexpr (std::is_same_v<T, TextureRef>) {
                const Texture* tex = p_materialSystem->GetTexture(v.handle);
                s.setInt("has_" + p.name, tex ? 1 : 0);
            if (tex) { tex->Bind(slot); s.setInt(p.name, slot++); }}
        }, p.value);
    }
}

const Texture* Renderer::FindMaterialTexture(const Material& m) const {
    for (const auto& p : m.props)
        if (const auto* t = std::get_if<TextureRef>(&p.value))
            if (const Texture* tex = p_materialSystem->GetTexture(t->handle)) return tex;
    return nullptr;
}