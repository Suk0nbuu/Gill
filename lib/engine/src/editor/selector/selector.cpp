#include "editor/selector/selector.hpp"
#include "component/mesh.hpp"
#include "core/system/mesh/mesh.hpp"
#include "render/shader/shader.hpp"
#include "scene/scene.hpp"
#include "core/system/transform/transform.hpp"




void Selector::Init(unsigned int width, unsigned int height) {
    up_selectShader = std::make_unique<Shader>("asset/shader/selectShader/selectVert.glsl","asset/shader/selectShader/selectFrag.glsl");
    m_width = width;
    m_height = height;
    glGenFramebuffers(1,&m_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER,m_FBO);

    glGenTextures(1,&m_Texture);
    glBindTexture(GL_TEXTURE_2D,m_Texture);
    glTexImage2D(GL_TEXTURE_2D,0,GL_R32I,m_width,m_height,0,GL_RED_INTEGER,GL_INT,nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,m_Texture,0);
    glGenRenderbuffers(1,&m_depthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER,m_depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT,width,height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,m_depthRenderBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
}



void Selector::RenderScene(const Scene* scene,TransformSystem* transformSystem,MeshSystem* meshSystem) {
    GLint clearValue = -1;
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    glClearBufferiv(GL_COLOR, 0, &clearValue);
    glClear(GL_DEPTH_BUFFER_BIT);
    up_selectShader->Use();
    scene->ForEach<comp::MeshComponent>([this, scene,transformSystem,meshSystem](Entity entity,const comp::MeshComponent& meshComp) {
       RenderEntityID(scene, entity, meshComp,transformSystem,meshSystem);
   });

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

std::optional<Entity> Selector::ReadEntityAt(int x, int y) const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    int flippedY = m_height - y;  // flip since glReadPixels origin is bottom-left

    GLint pickedID = -1;
    glReadPixels(x, flippedY, 1, 1, GL_RED_INTEGER, GL_INT, &pickedID);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
