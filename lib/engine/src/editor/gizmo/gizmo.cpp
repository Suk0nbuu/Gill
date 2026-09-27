#include "editor/gizmo/gizmo.hpp"
#include "core/debug/error.hpp"
#include <iostream>
#include <ostream>
#include "core/system/transform/transform.hpp"
#include "glad/gl.h"
#include "core/input/input.hpp"
#include "io/obj/obj.hpp"
#include "scene/scene.hpp"
#include "render/shader/shader.hpp"
#include <iostream>


void Gizmo::Init(unsigned int width,unsigned int height,TransformData* gizmoData,TransformSystem* transformSystem) {
    m_width = width;
    m_height = height;
    glEnable(GL_PROGRAM_POINT_SIZE);
    glGenVertexArrays(1, &m_originVAO);
    glBindVertexArray(m_originVAO);
    glGenBuffers(1, &m_originVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_originVBO);
    float origin[3] = {0.0f, 0.0f, 0.0f};
    glBufferData(GL_ARRAY_BUFFER, sizeof(origin), &origin[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    p_transformSystem = transformSystem;
    p_gizmoData = gizmoData;
    up_gizmoShader = std::make_unique<Shader>("asset/shader/gizmo/gizmoShader/gizmoVert.glsl", "asset/shader/gizmo/gizmoShader/gizmoFrag.glsl");
    up_gizmoShaderID = std::make_unique<Shader>("asset/shader/gizmo/gizmoShaderID/gizmoVertID.glsl","asset/shader/gizmo/gizmoShaderID/gizmoFragID.glsl");
    up_translateMesh = std::make_unique<Mesh>(LoadOBJ("asset/mesh/core/gizmo/GizmoTranslate.obj"));
    up_rotateMesh = std::make_unique<Mesh>(LoadOBJ("asset/mesh/core/gizmo/GizmoRotate.obj"));
    up_scaleMesh = std::make_unique<Mesh>(LoadOBJ("asset/mesh/core/gizmo/GizmoScale.obj"));
    up_planeMesh = std::make_unique<Mesh>(LoadOBJ("asset/mesh/core/gizmo/GizmoPlane.obj"));
    glGenFramebuffers(1,&m_pickFBO);
    glBindFramebuffer(GL_FRAMEBUFFER,m_pickFBO);

    glGenTextures(1,&m_pickTexture);
    glBindTexture(GL_TEXTURE_2D,m_pickTexture);
    glTexImage2D(GL_TEXTURE_2D,0,GL_R32I,m_width,m_height,0,GL_RED_INTEGER,GL_INT,nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,m_pickTexture,0);
    glGenRenderbuffers(1,&m_pickDepthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER,m_pickDepthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT,m_width,m_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,m_pickDepthRenderBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
}

void Gizmo::Render(Scene* scene,const mathpp::mat4f& view, const mathpp::mat4f& projection,const mathpp::vec3f& gizmoPosition, const mathpp::vec3f& cameraPos,Entity entity) {
    up_gizmoShader->Use();
    up_gizmoShader->setMat4f("view", view);
    up_gizmoShader->setMat4f("projection", projection);


    glDisable(GL_DEPTH_TEST);
    float distance = mathpp::length3v(cameraPos - gizmoPosition);
    float scale = distance * 0.08f;
    mathpp::mat4f rotY;
    mathpp::mat4f rotX;
    mathpp::mat4f rotZ;
    ComputeAxisRotations(entity, rotX, rotY, rotZ);

    mathpp::vec3f xColor = (em_highlightedAxis == TransformAxis::X) ? mathpp::vec3f(1.0f,1.0f,0.0f) : mathpp::vec3f(0.8f,0.0f,0.0f);
    DrawAxis(gizmoPosition, rotX, xColor, scale);

    mathpp::vec3f yColor = (em_highlightedAxis == TransformAxis::Y) ? mathpp::vec3f(0.0f,1.0f,1.0f) : mathpp::vec3f(0.0f,0.8f,0.0f);
    DrawAxis(gizmoPosition, rotY, yColor, scale);
    mathpp::vec3f zColor = (em_highlightedAxis == TransformAxis::Z) ? mathpp::vec3f(1.0f,0.0f,1.0f) : mathpp::vec3f(0.0f,0.0f,0.8f);
    DrawAxis(gizmoPosition, rotZ, zColor, scale);


    glEnable(GL_DEPTH_TEST);
}



void Gizmo::DrawAxis(const mathpp::vec3f &gizmoPosition, const mathpp::mat4f &axisRotation, const mathpp::vec3f &color, float scale) {
    up_gizmoShader->setVec3f("axisColor", color);
    mathpp::mat4f identity;
    Mesh* drawMesh = nullptr;
    if (p_gizmoData->mode == TransformMode::Translate){drawMesh = up_translateMesh.get();}
    else if (p_gizmoData->mode == TransformMode::Rotate){drawMesh = up_rotateMesh.get();}
    else if (p_gizmoData->mode == TransformMode::Scale){drawMesh = up_scaleMesh.get();}
    mathpp::mat4f meshModel = mathpp::translate(identity, gizmoPosition);
    meshModel = meshModel * axisRotation;
    if (p_gizmoData->mode != TransformMode::Rotate)
    {
    meshModel = mathpp::translate(meshModel, {0.0f, m_AxisHeight * 0.4f * scale, 0.0f});
        meshModel = meshModel = mathpp::scale(meshModel, {scale, scale, scale});

    }
    else{
        meshModel = meshModel = mathpp::scale(meshModel, {scale, scale, scale});
    }

    up_gizmoShader->setMat4f("model", meshModel);
    drawMesh->Draw();
}

void Gizmo::RenderIDs(const mathpp::mat4f &view, const mathpp::mat4f &projection, const mathpp::vec3f &gizmoPosition, const mathpp::vec3f &cameraPos,Entity entity) {
    GLint clearValue = 0;
    glBindFramebuffer(GL_FRAMEBUFFER, m_pickFBO);
    glClearBufferiv(GL_COLOR, 0, &clearValue);
    glClear(GL_DEPTH_BUFFER_BIT);
    up_gizmoShaderID->Use();
    up_gizmoShaderID->setMat4f("view", view);
    up_gizmoShaderID->setMat4f("projection", projection);

    glDisable(GL_DEPTH_TEST);

    float distance = mathpp::length3v(cameraPos - gizmoPosition);
    float scale = distance * 0.08f;

    mathpp::mat4f rotY;
    mathpp::mat4f rotX;
    mathpp::mat4f rotZ;

    ComputeAxisRotations(entity, rotX, rotY, rotZ);

    DrawAxisID(gizmoPosition, rotX, scale,static_cast<int>(TransformAxis::X));
    DrawAxisID(gizmoPosition, rotY,  scale,static_cast<int>(TransformAxis::Y));
    DrawAxisID(gizmoPosition, rotZ,  scale,static_cast<int>(TransformAxis::Z));

    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Gizmo::DrawAxisID(const mathpp::vec3f &gizmoPosition, const mathpp::mat4f &axisRotation, float scale, unsigned int ID) {
    up_gizmoShaderID->setInt("GizmoAxis",ID);
    mathpp::mat4f identity;
    Mesh* drawMesh = nullptr;
    if (p_gizmoData->mode == TransformMode::Translate){drawMesh = up_translateMesh.get();}
    else if (p_gizmoData->mode == TransformMode::Rotate){drawMesh = up_rotateMesh.get();}
    else if (p_gizmoData->mode == TransformMode::Scale){drawMesh = up_scaleMesh.get();}
    mathpp::mat4f axisModel = mathpp::translate(identity, gizmoPosition);
    axisModel = axisModel * axisRotation;
    if (p_gizmoData->mode != TransformMode::Rotate)
    {
        axisModel = mathpp::translate(axisModel, {0.0f, m_AxisHeight * 0.4f * scale, 0.0f});
        axisModel = axisModel = mathpp::scale(axisModel, {scale, scale, scale});

    }
    else{
        axisModel = axisModel = mathpp::scale(axisModel, {scale, scale, scale});
    }
    up_gizmoShaderID->setMat4f("model", axisModel);
    drawMesh->Draw();

}

TransformAxis Gizmo::ReadAxisAt(int x, int y) const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_pickFBO);
    int flippedY = m_height - y;  // flip since glReadPixels origin is bottom-left

    GLint pickedID = 0;
    glReadPixels(x, flippedY, 1, 1, GL_RED_INTEGER, GL_INT, &pickedID);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return static_cast<TransformAxis>(pickedID);
}



void Gizmo::UpdateHighlight(int x, int y, TransformAxis dragAxis, bool isDragging)  {
    if (isDragging) {
        em_highlightedAxis = dragAxis;
    }
    else {
        em_highlightedAxis = ReadAxisAt(x,y);
    }
}
Gizmo::~Gizmo() = default;

void Gizmo::ComputeAxisRotations(Entity entity, mathpp::mat4f& outRotX, mathpp::mat4f& outRotY, mathpp::mat4f& outRotZ) const {
    mathpp::quatf worldRot = (p_gizmoData->referenceFrame == ReferenceFrame::Local)
        ? p_transformSystem->GetWorldRotation(entity)
        : mathpp::quatf{};
    mathpp::mat4f worldRotMat = mathpp::QuatToMat4(worldRot);

    if (p_gizmoData->mode != TransformMode::Rotate) {
        outRotY = worldRotMat * mathpp::EulerAnglesRotation<float>({0.0f, 90.0f, 0.0f});
        outRotX = worldRotMat * mathpp::EulerAnglesRotation<float>({0.0f, 0.0f, -90.0f});
        outRotZ = worldRotMat * mathpp::EulerAnglesRotation<float>({90.0f, 0.0f, 0.0f});
    } else {
        outRotZ = worldRotMat * mathpp::EulerAnglesRotation<float>({0.0f, 90.0f, 0.0f});
        outRotY = worldRotMat * mathpp::EulerAnglesRotation<float>({0.0f, 0.0f, -90.0f});
        outRotX = worldRotMat * mathpp::EulerAnglesRotation<float>({90.0f, 0.0f, 0.0f});
    }
}

void Gizmo::DrawOriginMarker(const mathpp::mat4f& view, const mathpp::mat4f& projection, const mathpp::vec3f& gizmoPosition) {
    up_gizmoShader->Use();
    up_gizmoShader->setMat4f("view", view);
    up_gizmoShader->setMat4f("projection", projection);
    up_gizmoShader->setVec3f("axisColor", mathpp::vec3f(1.0f, 1.0f, 1.0f));

    mathpp::mat4f model = mathpp::translate(mathpp::mat4f(), gizmoPosition);
    up_gizmoShader->setMat4f("model", model);

    bool depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(m_originVAO);
    glPointSize(20.0f);
    glDrawArrays(GL_POINTS, 0, 1);

    if (depthWasEnabled) glEnable(GL_DEPTH_TEST);  // restore whatever state it found
}

