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
#include "core/system/hierarchy/hierarchy.hpp"
#include <type_traits>


void Renderer::Init(Scene* scene,TransformSystem* transformSystem,MeshSystem* meshSystem,MaterialSystem* materialSystem,ArmatureSystem* armatureSystem,Hierarchy* hierarchy) {
    glEnable(GL_DEPTH_TEST);
    p_scene = scene;
    p_transformSystem = transformSystem;
    p_meshSystem = meshSystem;
    p_materialSystem = materialSystem;
    p_armatureSystem = armatureSystem;
    p_hierarchy = hierarchy;
    up_lightSystem = std::make_unique<LightSystem>();
    up_lightSystem->Init(transformSystem);
    textureShader = std::make_unique<Shader>("asset/shader/textureShader/textureVert.glsl","asset/shader/textureShader/textureFrag.glsl");
    solidShader = std::make_unique<Shader>("asset/shader/solidShader/solidVert.glsl","asset/shader/solidShader/solidFrag.glsl");
    matCapTexture = std::make_unique<Texture>("asset/texture/core/SolidTex2.png");
    fallBackTexture = std::make_unique<Texture>("asset/texture/core/Debugempty.png");
    up_cameraUBO = std::make_unique<UniformBuffer>(sizeof(CameraBlock), kCameraBinding);
    up_lightUBO = std::make_unique<UniformBuffer>(sizeof(LightBlock), kLightBinding);
    up_skinUBO = std::make_unique<UniformBuffer>(sizeof(mathpp::mat4f) * kMaxBones, kSkinBinding);
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

        if (features & Feature_Skinning) UploadSkinningPalette(scene, entity);
    } else if (em_viewportMode == ViewportMode::Rendered) {
        Shader* shader = nullptr;
        if (mat && mat->graph && mat->graphShaderId >= 0) {
            const CodegenResult& prog = p_materialSystem->GetGeneratedProgram(mat->graphShaderId);
            Shader* g = m_shaderCache.GetGenerated(kSurfaceVert, mat->graphShaderId, prog.fragSource, features);
            if (g->IsValid()) { g->Use(); UploadGraphMaterial(*mat, prog, *g); shader = g; }
        } else if (mat) {
            if (const ShaderDef* def = p_materialSystem->GetShaderDef(mat->shaderKey)) {
                Shader* s = m_shaderCache.Get(def->vert, def->frag, features);
                if (s->IsValid()) { s->Use(); UploadMaterial(*mat, *s); shader = s; }
            }
        }
        if (!shader) { solidShader->Use(); shader = solidShader.get(); }   // missing or broken shader: same fallback as before

        shader->setMat4f("model", p_transformSystem->GetWorldTransform(entity));
        if (features & Feature_Skinning) UploadSkinningPalette(scene, entity);
    }
    else {
        Shader* shader = m_shaderCache.Get("asset/shader/textureShader/textureVert.glsl", "asset/shader/textureShader/textureFrag.glsl", features);
        shader->Use();
        const Texture* tex = mat ? FindMaterialTexture(*mat) : nullptr;
        if (!tex) tex = fallBackTexture.get();
        tex->Bind(0);
        shader->setInt("meshTexture", 0);
        shader->setMat4f("model", p_transformSystem->GetWorldTransform(entity));

        if (features & Feature_Skinning) UploadSkinningPalette(scene, entity);
    }

    mesh->Draw();
}

Renderer::~Renderer() = default;

void Renderer::UploadSkinningPalette(const Scene* scene, Entity entity) {
    std::array<mathpp::mat4f, kMaxBones> block;
    block.fill(mathpp::mat4f());

    if (const auto* armComp = scene->TryGetComponent<comp::ArmatureComponent>(entity)) {
        const std::vector<mathpp::mat4f> palette = p_armatureSystem->ComputeSkinningPalette(*armComp);
        const size_t n = std::min(palette.size(), static_cast<size_t>(kMaxBones));
        std::copy_n(palette.begin(), n, block.begin());
    }
    up_skinUBO->Update(block.data(), sizeof(mathpp::mat4f) * kMaxBones);
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

const comp::ArmatureComponent* Renderer::FindArmature(const Scene* scene, Entity e) const {
    for (int depth = 0; depth < 64; ++depth) {
        if (const auto* a = scene->TryGetComponent<comp::ArmatureComponent>(e)) return a;
        const auto parent = p_hierarchy->TryGetParent(e);
        if (!parent) return nullptr;
        e = *parent;
    }
    return nullptr;
}

void Renderer::UploadGraphMaterial(const Material& m, const CodegenResult& prog, const Shader& s) const {
    const ShaderGraph& g = *m.graph;

    for (const auto& u : prog.uniforms) {
        const GraphNode* n = g.Find(u.nodeId);
        if (!n || u.socketIndex >= static_cast<int>(n->inputValues.size())) continue;
        const SocketValue& v = n->inputValues[u.socketIndex];
        if (u.type == SocketType::Float) {
            if (const float* f = std::get_if<float>(&v)) s.setFloat(u.name, *f);
        } else {
            if (const mathpp::vec3f* c = std::get_if<mathpp::vec3f>(&v)) s.setVec3f(u.name, *c);
        }
    }

    int slot = 0;
    for (const auto& t : prog.textures) {
        const GraphNode* n = g.Find(t.nodeId);
        const Texture* tex = n ? p_materialSystem->GetTexture(n->image.handle) : nullptr;
        s.setInt("has_" + t.name, tex ? 1 : 0);
        if (tex) { tex->Bind(slot); s.setInt(t.name, slot++); }
    }
}