#include "editor/selector/selector.hpp"
#include "component/mesh.hpp"
#include "core/system/mesh/mesh.hpp"
#include "render/shader/shader.hpp"
#include "scene/scene.hpp"
#include "core/system/transform/transform.hpp"
#include "editor/core/fboScope.hpp"
#include <algorithm>




void Selector::Init(unsigned int width, unsigned int height) {
    up_selectShader = std::make_unique<Shader>("asset/shader/selectShader/selectVert.glsl","asset/shader/selectShader/selectFrag.glsl");
    glGenFramebuffers(1,&m_FBO);
    glGenTextures(1,&m_Texture);
    glGenRenderbuffers(1,&m_depthRenderBuffer);
    Resize(width, height);
}

void Selector::Resize(unsigned int width, unsigned int height) {
    width = std::max(width, 1u);
    height = std::max(height, 1u);
    if (width == m_width && height == m_height) return;
    m_width = width;
    m_height = height;

    FboScope scope;
    glBindFramebuffer(GL_FRAMEBUFFER,m_FBO);
    glBindTexture(GL_TEXTURE_2D,m_Texture);
    glTexImage2D(GL_TEXTURE_2D,0,GL_R32I,m_width,m_height,0,GL_RED_INTEGER,GL_INT,nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,m_Texture,0);
    glBindRenderbuffer(GL_RENDERBUFFER,m_depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT,m_width,m_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,m_depthRenderBuffer);
}

void Selector::RenderScene(const Scene* scene,TransformSystem* transformSystem,MeshSystem* meshSystem) {
    FboScope scope;                                  // restores the caller's FBO and viewport
    GLint clearValue = -1;
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    glViewport(0, 0, m_width, m_height);
    glClearBufferiv(GL_COLOR, 0, &clearValue);
    glClear(GL_DEPTH_BUFFER_BIT);
    up_selectShader->Use();
    scene->ForEach<comp::MeshComponent>([this, scene,transformSystem,meshSystem](Entity entity,const comp::MeshComponent& meshComp) {
       RenderEntityID(scene, entity, meshComp,transformSystem,meshSystem);
   });
}

std::optional<Entity> Selector::ReadEntityAt(int x, int y) const {
    // x, y are viewport-local pixels, origin top-left
    if (x < 0 || y < 0 || x >= static_cast<int>(m_width) || y >= static_cast<int>(m_height)) return std::nullopt;

    FboScope scope;
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    int flippedY = static_cast<int>(m_height) - 1 - y;  // glReadPixels origin is bottom-left

    GLint pickedID = -1;
    glReadPixels(x, flippedY, 1, 1, GL_RED_INTEGER, GL_INT, &pickedID);

    if (pickedID == -1)
    {return std::nullopt;}

    return static_cast<Entity>(pickedID);
}

void Selector::RenderEntityID(const Scene *scene,Entity entity, const comp::MeshComponent &meshComp,TransformSystem* transformSystem,MeshSystem* meshSystem) {
    auto mesh = meshSystem->GetMesh(meshComp.meshID);
    up_selectShader->setMat4f("model",transformSystem->GetWorldTransform(entity) );
    up_selectShader->setInt("ObjectID", static_cast<int>(entity));
    mesh->Draw();


}
