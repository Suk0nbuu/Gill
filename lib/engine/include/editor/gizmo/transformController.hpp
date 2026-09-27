#pragma once
#include "gizmo.hpp"
#include "mathpp.hpp"
#include "transformData.hpp"
#include <unordered_map>
#include <unordered_set>


class TransformSystem;
class Hierarchy;
class SelectionManager;
using Entity = uint32_t;
struct Ray;

class TransformController {
public:
    ~TransformController();
    void Init(float width, float height,TransformData* gizmoData,TransformSystem* transformSystem,SelectionManager* selectionManager,Hierarchy* hierarchy);
    void Begin(const mathpp::mat4f& view, const mathpp::mat4f& proj, float mouseX, float mouseY);
    bool Apply(const mathpp::mat4f& view, const mathpp::mat4f& proj,float mouseX, float mouseY);
    void Cancel();
    TransformAxis GetActiveAxis() const;
    bool IsDragging();
    void End();
    TransformMode GetMode();
    void SetMode(const TransformMode& mode);

private:
    bool IntersectPlane(const mathpp::vec3f& planeNormal, const mathpp::vec3f& planePoint,const Ray& ray, float& outT) const;
    void ComputeNDC(float &x, float &y,float mouseX,float mouseY) const ;
    mathpp::vec3f ComputeMedianPos(const std::unordered_set<Entity>& selected);
    mathpp::vec3f GetAxis();
    mathpp::vec3f GetAxisFor(Entity entity);
    mathpp::vec3f GetCameraPosition(const mathpp::mat4f& view) const ;
    mathpp::vec3f GetViewPlaneNormal(const mathpp::mat4f& view) const;
    mathpp::vec3f GetRotateNormal(const mathpp::mat4f& view) const;
    bool ProjectRayOntoAxis(const mathpp::vec3f& axisDir, const Ray& ray, float& outT) const;
    Ray GetMouseRay(const mathpp::mat4f& view, const mathpp::mat4f& proj, float mouseX, float mouseY) const;;
    bool ContinueTranslate(const mathpp::mat4f& view,const mathpp::mat4f& proj,float mouseX,float mouseY,float& outT);
    void EnterMode(TransformMode mode, const mathpp::mat4f& view, const mathpp::mat4f& proj, float mouseX, float mouseY);
    bool ContinueScale(const mathpp::mat4f& view,const mathpp::mat4f& proj,float mouseX,float mouseY,mathpp::vec3f& outValue);
    bool ContinueRotate(const mathpp::mat4f& view,const mathpp::mat4f& proj,float mouseX,float mouseY,mathpp::quatf& outValue);
    bool ContinueTranslateFree(const mathpp::mat4f& view, const mathpp::mat4f& proj, float mouseX, float mouseY, mathpp::vec3f& outDelta);
    void HandleAxisKey(TransformAxis axis, const mathpp::mat4f& view, const mathpp::mat4f& proj, float mouseX, float mouseY);
    void RebaseDragStart(const mathpp::mat4f& view, const mathpp::mat4f& proj, float mouseX, float mouseY) ;
    bool isDragging = false;
    TransformData* p_gizmoData;
    TransformSystem* p_transformSystem;
    SelectionManager* p_selectionManager;
    Hierarchy* p_hierarchy;
    mathpp::vec3f m_pivotStartPos;
    mathpp::vec3f m_dragStartRadial;
    mathpp::vec3f m_dragStartPlaneHit;
    float m_scaleStartT;
    std::unordered_map<Entity,TransformSnapshot> um_worldTransforms;
    float m_width, m_height;
    mathpp::vec3f m_transformAxis;
    static constexpr float minDenom = 0.01f;
    static constexpr float sensitivity = 0.4f;
};



