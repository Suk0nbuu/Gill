#pragma once
#include "mathpp.hpp"
#include "transformData.hpp"
#include <memory>

class Scene;
class Mesh;
class Shader;
using Entity = uint32_t;
class TransformSystem;


class Gizmo {
public:

    void Init(unsigned int width,unsigned int height,TransformData* gizmoData,TransformSystem* transformSystem);
    void Render(Scene* scene,const mathpp::vec3f& gizmoPosition, const mathpp::vec3f& cameraPos,Entity entity);
    void RenderIDs(const mathpp::vec3f& gizmoPosition, const mathpp::vec3f& cameraPos, Entity entity);
    void UpdateHighlight(int x, int y,TransformAxis dragAxis,bool isDragging = false);
    void DrawOriginMarker(const mathpp::vec3f& gizmoPosition);
    TransformAxis ReadAxisAt(int x, int y) const;
    ~Gizmo();

private:
    void DrawAxis(const mathpp::vec3f& gizmoPosition, const mathpp::mat4f& axisRotation,const mathpp::vec3f& color, float scale);
    void DrawAxisID(const mathpp::vec3f& gizmoPosition, const mathpp::mat4f& axisRotation, float scale, unsigned int ID);
    void ComputeAxisRotations(Entity entity, mathpp::mat4f& outRotX, mathpp::mat4f& outRotY, mathpp::mat4f& outRotZ) const;
    std::unique_ptr<Mesh> up_translateMesh;
    std::unique_ptr<Mesh> up_rotateMesh;
    std::unique_ptr<Mesh> up_scaleMesh;
    std::unique_ptr<Mesh> up_planeMesh;
    std::unique_ptr<Shader> up_gizmoShader;
    std::unique_ptr<Shader> up_gizmoShaderID;
    unsigned int m_pickFBO, m_pickTexture, m_pickDepthRenderBuffer;
    unsigned int m_pickWidth, m_pickHeight;
    unsigned int m_originVBO,m_originVAO;
    float m_width,m_height;
    float m_AxisHeight = 1.0f;
    TransformData* p_gizmoData;
    TransformAxis em_highlightedAxis = TransformAxis::None;
    TransformSystem* p_transformSystem;
};