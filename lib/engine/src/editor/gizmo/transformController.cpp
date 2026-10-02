#include "editor/gizmo/transformController.hpp"
#include "core/component/raycast/raycast.hpp"
#include "core/system/transform/transform.hpp"
#include "editor/selector/selectionManager.hpp"
#include "core/system/hierarchy/hierarchy.hpp"


TransformAxis TransformController::GetActiveAxis() const {
    return p_gizmoData->axis;
}

TransformController::~TransformController() = default;

void TransformController::Init(float width, float height,TransformData* gizmoData,TransformSystem* transformSystem,SelectionManager* selectionManager,Hierarchy* hierarchy) {
    m_width = width;
    m_height = height;
    p_gizmoData = gizmoData;
    p_transformSystem = transformSystem;
    p_selectionManager = selectionManager;
    p_hierarchy = hierarchy;
}

bool TransformController::IsDragging() {
    return isDragging;
}

void TransformController::Begin(const mathpp::mat4f& view, const mathpp::mat4f& proj, float mouseX, float mouseY) {
    if (isDragging) return;
    if (!p_selectionManager->GetActiveSelected().has_value()) return;
    m_screenCorrection = {0.0f, 0.0f};
    m_transformAxis = GetAxis();
    auto selected = p_selectionManager->GetAllSelected();
    m_pivotStartPos = ComputeMedianPos(selected);

    for (auto entity : selected) {
        const auto& worldTransform = p_transformSystem->GetWorldTransform(entity);
        TransformSnapshot snapshot;
        snapshot.worldPos = mathpp::TranslateFromMat4(worldTransform);
        snapshot.worldRot = p_transformSystem->GetWorldRotation(entity);
        snapshot.worldScale = mathpp::ScaleFromMat4(worldTransform);
        um_worldTransforms.insert({entity, snapshot});
    }

    Ray ray = GetMouseRay(view, proj, mouseX, mouseY);

    switch (p_gizmoData->mode) {
        case TransformMode::Rotate: {
            float t{};
            if (IntersectPlane(GetRotateNormal(view), m_pivotStartPos, ray, t)) {
                m_dragStartRadial = ray.origin + ray.direction * t - m_pivotStartPos;
            }
            break;
        }
        case TransformMode::Scale: {
            if (p_gizmoData->axis == TransformAxis::None) {
                float t{};
                if (IntersectPlane(GetViewPlaneNormal(view), m_pivotStartPos, ray, t)) {
                    mathpp::vec3f hitPoint = ray.origin + ray.direction * t;
                    m_scaleStartT = mathpp::length3v(hitPoint - m_pivotStartPos);
                }
            } else {
                float t{};
                if (ProjectRayOntoAxis(m_transformAxis, ray, t)) m_scaleStartT = t;
            }
            break;
        }
        case TransformMode::Translate: {
            if (p_gizmoData->axis == TransformAxis::None) {
                float t{};
                if (IntersectPlane(GetViewPlaneNormal(view), m_pivotStartPos, ray, t)) {
                    m_dragStartPlaneHit = ray.origin + ray.direction * t;
                }
            }
            break;
        }
    }

    isDragging = true;
}

mathpp::vec3f TransformController::GetCameraPosition(const mathpp::mat4f& view) const {
    return mathpp::TranslateFromMat4(mathpp::inverse(view));
}

void TransformController::ComputeNDC(float &x, float &y,float mouseX,float mouseY) const  {
    x =(mouseX/m_width)*2.0f - 1.0f;
    y = 1.0f - (mouseY/m_height)*2.0f;
}

bool TransformController::Apply(const mathpp::mat4f &view, const mathpp::mat4f &proj, float mouseX, float mouseY) {
    switch (p_gizmoData->mode) {
        // Apply()'s Translate case — branch on axis
        case TransformMode::Translate: {
            mathpp::vec3f delta;
            if (p_gizmoData->axis == TransformAxis::None) {
                if (!ContinueTranslateFree(view, proj, mouseX, mouseY, delta)) return false;
            } else {
                float t;
                if (!ContinueTranslate(view, proj, mouseX, mouseY, t)) return false;
                delta = m_transformAxis * t;
            }

            for (auto& [entity, snapshot] : um_worldTransforms) {
                mathpp::vec3f newWorldPos = (p_gizmoData->axis == TransformAxis::None)
                    ? snapshot.worldPos + delta
                    : snapshot.worldPos + GetAxisFor(entity) * mathpp::dot(delta, m_transformAxis);

                mathpp::mat4f invParentWorld = mathpp::inverse(p_transformSystem->GetParentWorldTransform(entity));
                mathpp::vec4f newWorldPos4 = {newWorldPos.x, newWorldPos.y, newWorldPos.z, 1.0f};
                mathpp::vec3f localPos = (invParentWorld * newWorldPos4).xyz();
                p_transformSystem->SetPosition(entity, localPos);
            }
            return true;
        }
        case TransformMode::Rotate: {
            mathpp::quatf deltaQuat;
            if (!ContinueRotate(view, proj, mouseX, mouseY, deltaQuat)) return false;

            for (auto& [entity, snapshot] : um_worldTransforms) {
                mathpp::vec3f newWorldPos;

                mathpp::quatf newWorldRot;

                switch (p_gizmoData->referenceFrame) {
                    case ReferenceFrame::World: {
                        // orbit around the shared multi-select pivot, delta expressed in world axes
                        mathpp::vec3f offset = snapshot.worldPos - m_pivotStartPos;
                        mathpp::vec3f rotatedOffset = mathpp::RotateVector(deltaQuat, offset);
                        newWorldPos = m_pivotStartPos + rotatedOffset;
                        newWorldRot = deltaQuat * snapshot.worldRot; // pre-multiply: rotate in world frame
                        break;
                    }
                    case ReferenceFrame::Local: {
                        // spin each object about its own origin, delta expressed in its own axes
                        newWorldPos = snapshot.worldPos; // origin doesn't move
                        newWorldRot = snapshot.worldRot * deltaQuat; // post-multiply: rotate in local frame
                        break;
                    }
                }

                if (p_hierarchy->HasParent(entity)) {
                    mathpp::quatf parentWorldRot = p_transformSystem->GetParentWorldRotation(entity);
                    mathpp::quatf parentWorldRotInv = mathpp::ConjugateQuat(parentWorldRot);

                    mathpp::mat4f parentWorldMat = p_transformSystem->GetParentWorldTransform(entity);
                    mathpp::vec3f parentWorldPos = mathpp::TranslateFromMat4(parentWorldMat);
                    mathpp::vec3f parentWorldScale = mathpp::ScaleFromMat4(parentWorldMat);

                    mathpp::vec3f rotatedOffset = mathpp::RotateVector(parentWorldRotInv, newWorldPos - parentWorldPos);
                    mathpp::vec3f newLocalPos = rotatedOffset / parentWorldScale; // component-wise divide

                    mathpp::quatf newLocalRot = parentWorldRotInv * newWorldRot; // rotation is unaffected by scale

                    p_transformSystem->SetPosition(entity, newLocalPos);
                    p_transformSystem->SetRotation(entity, newLocalRot);
                }
                else {
                    p_transformSystem->SetPosition(entity,newWorldPos);
                    p_transformSystem->SetRotation(entity,newWorldRot);
                }
            }
            return true;
        }
        case TransformMode::Scale: {
            mathpp::vec3f deltaScale; // component-wise scale factor from the drag, e.g. (1.5, 1, 1)
            if (!ContinueScale(view, proj, mouseX, mouseY, deltaScale)) return false;

        for (auto& [entity, snapshot] : um_worldTransforms) {
            mathpp::vec3f newWorldPos;
            mathpp::vec3f newWorldScale;

            switch (p_gizmoData->referenceFrame) {
                case ReferenceFrame::World: {
                // reposition relative to shared pivot, scaled along world axes
                mathpp::vec3f offset = snapshot.worldPos - m_pivotStartPos;
                mathpp::vec3f scaledOffset = offset * deltaScale; // component-wise
                newWorldPos = m_pivotStartPos + scaledOffset;
                newWorldScale = snapshot.worldScale * deltaScale; // component-wise
                break;
                }
                case ReferenceFrame::Local: {
                // scale in place along the object's own axes
                newWorldPos = snapshot.worldPos;
                newWorldScale = snapshot.worldScale * deltaScale; // component-wise
                break;
                }
            }

            // parent-relative conversion (position needs full T*R*S inverse;
            // scale conversion is just component-wise divide by parent's world scale)
            if (p_hierarchy->HasParent(entity)) {
                mathpp::quatf parentWorldRot = p_transformSystem->GetParentWorldRotation(entity);
                mathpp::quatf parentWorldRotInv = mathpp::ConjugateQuat(parentWorldRot);

                mathpp::mat4f parentWorldMat = p_transformSystem->GetParentWorldTransform(entity);
                mathpp::vec3f parentWorldPos = mathpp::TranslateFromMat4(parentWorldMat);
                mathpp::vec3f parentWorldScale = mathpp::ScaleFromMat4(parentWorldMat);

                mathpp::vec3f rotatedOffset = mathpp::RotateVector(parentWorldRotInv, newWorldPos - parentWorldPos);
                mathpp::vec3f newLocalPos = rotatedOffset / parentWorldScale;
                mathpp::vec3f newLocalScale = newWorldScale / parentWorldScale;

                p_transformSystem->SetPosition(entity, newLocalPos);
                p_transformSystem->SetScale(entity, newLocalScale);
            } else {
                p_transformSystem->SetPosition(entity, newWorldPos);
                p_transformSystem->SetScale(entity, newWorldScale);
            }
        }
            return true;
        }
    }
    return false;
}


void TransformController::End() {
    isDragging = false;
    p_gizmoData->axis =TransformAxis::None;
    um_worldTransforms.clear();
}

void TransformController::SetMode(const TransformMode &mode) {
    p_gizmoData->mode = mode;
}

TransformMode TransformController::GetMode() {
    return p_gizmoData->mode;
}

bool TransformController::ContinueTranslate(const mathpp::mat4f &view, const mathpp::mat4f &proj, float mouseX, float mouseY, float& outT) {
    Ray ray = GetMouseRay(view, proj, mouseX, mouseY);
    return ProjectRayOntoAxis(m_transformAxis, ray, outT);
}

bool TransformController::ContinueScale(const mathpp::mat4f &view, const mathpp::mat4f &proj, float mouseX, float mouseY, mathpp::vec3f &outValue) {
    Ray ray = GetMouseRay(view, proj, mouseX, mouseY);

    if (p_gizmoData->axis == TransformAxis::None) {
        float t{};
        if (!IntersectPlane(GetViewPlaneNormal(view), m_pivotStartPos, ray, t)) return false;
        mathpp::vec3f hitPoint = ray.origin + ray.direction * t;
        float currentLen = mathpp::length3v(hitPoint - m_pivotStartPos);

        if (m_scaleStartT <= minDenom) return false;
        float multiplier = currentLen / m_scaleStartT;
        if (multiplier < 0.001f) multiplier = 0.001f;

        outValue = mathpp::vec3f(multiplier, multiplier, multiplier);
        return true;
    }

    float t;
    if (!ProjectRayOntoAxis(m_transformAxis, ray, t)) return false;
    if (std::abs(m_scaleStartT) <= minDenom) return false;

    float multiplier = t / m_scaleStartT;
    if (std::abs(multiplier) < 0.001f) multiplier = multiplier < 0 ? -0.001f : 0.001f;

    outValue = mathpp::vec3f(1.0f, 1.0f, 1.0f);
    switch (p_gizmoData->axis) {
        case TransformAxis::X: outValue.x = multiplier; break;
        case TransformAxis::Y: outValue.y = multiplier; break;
        case TransformAxis::Z: outValue.z = multiplier; break;
        default: break;
    }
    return true;
}

bool TransformController::ContinueRotate(const mathpp::mat4f& view, const mathpp::mat4f& proj, float mouseX, float mouseY, mathpp::quatf& outDelta) {
    mathpp::vec3f rotateNormal = GetRotateNormal(view);
    Ray ray = GetMouseRay(view, proj, mouseX, mouseY);

    float t{};
    if (!IntersectPlane(rotateNormal, m_pivotStartPos, ray, t)) return false;
    mathpp::vec3f hitPoint = ray.origin + ray.direction * t;
    mathpp::vec3f currentRadial = hitPoint - m_pivotStartPos;

    float dotVal = mathpp::dot(m_dragStartRadial, currentRadial);
    mathpp::vec3f crossVal = mathpp::cross(m_dragStartRadial, currentRadial);
    float deltaTheta = atan2(mathpp::dot(crossVal, rotateNormal), dotVal);
    float halfAngle = deltaTheta / 2.0f;
    float cosH = cosf(halfAngle);
    float sinH = sinf(halfAngle);

    outDelta = {cosH, rotateNormal.x * sinH, rotateNormal.y * sinH, rotateNormal.z * sinH};
    return true;
}
bool TransformController::IntersectPlane(const mathpp::vec3f& planeNormal, const mathpp::vec3f& planePoint,const Ray& ray, float& outT) const {
    float denom = mathpp::dot(planeNormal, ray.direction);
    if (std::abs(denom) <= minDenom) return false;
    outT = mathpp::dot(planePoint - ray.origin, planeNormal) / denom;
    return true;
}



mathpp::vec3f TransformController::ComputeMedianPos(const std::unordered_set<Entity>& selected) {
    mathpp::vec3f sum;
    float size = static_cast<float>(selected.size());
    for (auto entity : selected) {
        mathpp::mat4f worldTransform = p_transformSystem->GetWorldTransform(entity);
        mathpp::vec3f worldPos = mathpp::TranslateFromMat4(worldTransform);
        sum += worldPos;
    }
    return sum/size;
}

mathpp::vec3f TransformController::GetAxisFor(Entity entity) {
    mathpp::vec3f axisDir;
    switch (p_gizmoData->axis) {
        case TransformAxis::Z: axisDir = {0.0f, 0.0f, 1.0f}; break;
        case TransformAxis::X: axisDir = {1.0f, 0.0f, 0.0f}; break;
        case TransformAxis::Y: axisDir = {0.0f, 1.0f, 0.0f}; break;
        default: axisDir = {0.0f, 0.0f, 0.0f}; break; // None: no single axis — callers must branch before using this
    }
    if (p_gizmoData->referenceFrame == ReferenceFrame::Local) {
        axisDir = mathpp::RotateVector(p_transformSystem->GetWorldRotation(entity), axisDir);
    }
    return axisDir;
}

mathpp::vec3f TransformController::GetAxis() {
    return GetAxisFor(p_selectionManager->GetActiveSelected().value());
}

void TransformController::Cancel() {
    if (!isDragging) return;
    for (auto& [entity, snapshot] : um_worldTransforms) {
        mathpp::mat4f invParentWorld = mathpp::inverse(p_transformSystem->GetParentWorldTransform(entity));
        mathpp::vec4f worldPos4 = {snapshot.worldPos.x, snapshot.worldPos.y, snapshot.worldPos.z, 1.0f};
        mathpp::vec3f localPos = (invParentWorld * worldPos4).xyz();

        mathpp::quatf parentWorldRot = p_transformSystem->GetParentWorldRotation(entity);
        mathpp::quatf localRot = mathpp::ConjugateQuat(parentWorldRot) * snapshot.worldRot;

        mathpp::vec3f parentWorldScale = mathpp::ScaleFromMat4(p_transformSystem->GetParentWorldTransform(entity));
        mathpp::vec3f localScale = snapshot.worldScale / parentWorldScale;

        p_transformSystem->SetPosition(entity, localPos);
        p_transformSystem->SetRotation(entity, localRot);
        p_transformSystem->SetScale(entity, localScale);
    }
    End();
}


bool TransformController::ContinueTranslateFree(const mathpp::mat4f& view, const mathpp::mat4f& proj, float mouseX, float mouseY, mathpp::vec3f& outDelta) {
    Ray ray = GetMouseRay(view, proj, mouseX, mouseY);
    float t{};
    if (!IntersectPlane(GetViewPlaneNormal(view), m_pivotStartPos, ray, t)) return false;
    mathpp::vec3f hitPoint = ray.origin + ray.direction * t;
    outDelta = hitPoint - m_dragStartPlaneHit;
    return true;
}

void TransformController::EnterMode(TransformMode mode, const mathpp::mat4f& view, const mathpp::mat4f& proj, float mouseX, float mouseY) {
    if (isDragging) return;
    if (!p_selectionManager->GetActiveSelected().has_value()) return;
    p_gizmoData->mode = mode;
    p_gizmoData->axis = TransformAxis::None;
    p_gizmoData->referenceFrame = ReferenceFrame::World;
    Begin(view, proj, mouseX, mouseY);
}


void TransformController::HandleAxisKey(TransformAxis axis, const mathpp::mat4f& view, const mathpp::mat4f& proj, float mouseX, float mouseY) {
    if (p_gizmoData->axis == axis) {
        ToggleReferenceFrame(p_gizmoData); // reuse the free function from transformData.hpp
    } else {
        p_gizmoData->axis = axis;
        p_gizmoData->referenceFrame = ReferenceFrame::World;
    }
    m_transformAxis = GetAxis();
    RebaseDragStart(view, proj, mouseX, mouseY);
}

void TransformController::RebaseDragStart(const mathpp::mat4f& view, const mathpp::mat4f& proj, float mouseX, float mouseY) {
    Ray ray = GetMouseRay(view, proj, mouseX, mouseY);
    if (p_gizmoData->mode == TransformMode::Rotate) {
        float t{};
        if (IntersectPlane(m_transformAxis, m_pivotStartPos, ray, t)) {
            m_dragStartRadial = ray.origin + ray.direction * t - m_pivotStartPos;
        }
    } else if (p_gizmoData->mode == TransformMode::Scale) {
        float t{};
        if (ProjectRayOntoAxis(m_transformAxis, ray, t)) m_scaleStartT = t;
    }
}



Ray TransformController::GetMouseRay(const mathpp::mat4f& view, const mathpp::mat4f& proj, float mouseX, float mouseY) const {
    float ndcX{}, ndcY{};
    ComputeNDC(ndcX, ndcY, mouseX + m_screenCorrection.x, mouseY + m_screenCorrection.y);
    return ScreenToRay(ndcX, ndcY, view, proj);
}

mathpp::vec3f TransformController::GetViewPlaneNormal(const mathpp::mat4f& view) const {
    return mathpp::normalize(m_pivotStartPos - GetCameraPosition(view));
}

mathpp::vec3f TransformController::GetRotateNormal(const mathpp::mat4f& view) const {
    return (p_gizmoData->axis == TransformAxis::None) ? GetViewPlaneNormal(view) : m_transformAxis;
}

bool TransformController::ProjectRayOntoAxis(const mathpp::vec3f& axisDir, const Ray& ray, float& outT) const {
    mathpp::vec3f w0 = m_pivotStartPos - ray.origin;
    float b = mathpp::dot(axisDir, ray.direction);
    float d = mathpp::dot(axisDir, w0);
    float e = mathpp::dot(ray.direction, w0);
    float denom = 1 - b * b;
    if (denom <= minDenom) return false;
    outT = (b * e - d) / denom;
    return true;
}
void TransformController::AddScreenCorrection(const mathpp::vec2f& delta) {
    m_screenCorrection += delta;
}
