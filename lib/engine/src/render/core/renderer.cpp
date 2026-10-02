#include "render/core/renderer.hpp"
#include "render/shader/shader.hpp"
#include "render/texture/texture.hpp"
#include "core/system/transform/transform.hpp"
#include "core/system/mesh/mesh.hpp"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "core/system/armature/armature.hpp"
#include "component/armature.hpp"
#include "core/system/material/material.hpp"
#include "scene/scene.hpp"

void Renderer::Init(TransformSystem* transformSystem,MeshSystem* meshSystem,MaterialSystem* materialSystem,ArmatureSystem* armatureSystem) {
    glEnable(GL_DEPTH_TEST);
    p_transformSystem = transformSystem;
    p_meshSystem = meshSystem;
    p_materialSystem = materialSystem;
    p_armatureSystem = armatureSystem;
    textureShader = std::make_unique<Shader>("asset/shader/textureShader/textureVert.glsl","asset/shader/textureShader/textureFrag.glsl");
    solidShader = std::make_unique<Shader>("asset/shader/solidShader/solidVert.glsl","asset/shader/solidShader/solidFrag.glsl");
    matCapTexture = std::make_unique<Texture>("asset/texture/core/SolidTex2.png");
    fallBackTexture = std::make_unique<Texture>("asset/texture/core/Debugempty.png");

}

void Renderer::SetViewportMode(ViewportMode mode) {
    em_viewportMode = mode;
}

ViewportMode Renderer::GetViewportMode() {
    return em_viewportMode;
}

void Renderer::renderScene(const Scene* scene, const mathpp::mat4f& view, const mathpp::mat4f& projection,const mathpp::vec3f& viewVec) {
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
        shader->setMat4f("view", view);
        shader->setMat4f("projection", proj);
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
        shader->setMat4f("view", view);
        shader->setMat4f("projection", proj);
        shader->setMat4f("model", p_transformSystem->GetWorldTransform(entity));
        shader->setVec3f("albedo", mat ? mat->albedo : mathpp::vec3f{1.0f,1.0f,1.0f});
        shader->setVec3f("viewVec", viewVec);
        shader->setVec3f("lightDir", mathpp::vec3f{0.8f,0.2f,0.0f});
    }
    else {
        Shader* shader = m_shaderCache.Get("asset/shader/textureShader/textureVert.glsl", "asset/shader/textureShader/textureFrag.glsl", features);
        shader->Use();
        const Texture* tex = mat ? p_materialSystem->GetTexture(mat->textureID) : nullptr;
        if (!tex) tex = fallBackTexture.get();
        tex->Bind(0);
        shader->setInt("meshTexture", 0);
        shader->setMat4f("view", view);
        shader->setMat4f("projection", proj);
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