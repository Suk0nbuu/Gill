#pragma once
#include "mathpp.hpp"

class Input;

enum class CameraType {
    Free,
    Orbit
};

// Interface
class ICamera {
public:
    virtual ~ICamera() = default;

    virtual void Update(Input* input, float deltaTime, const mathpp::vec3f& target, bool suppressLook) = 0;
    virtual mathpp::vec3f GetPosition() const = 0;
    virtual mathpp::mat4f GetViewMatrix() const = 0;
    virtual bool IsDragging(Input* input) const  = 0;
};

// Free-fly camera
class FreeCamera : public ICamera {
public:
    void Update(Input* input, float deltaTime, const mathpp::vec3f& target, bool suppressLook) override;
    mathpp::vec3f GetPosition() const override;
    mathpp::mat4f GetViewMatrix() const override;
    bool IsDragging(Input* input) const override;

private:
    mathpp::vec3f m_position;
    mathpp::vec3f m_camFront;
    mathpp::mat4f m_viewMatrix;

    float m_yaw = -90.0f;
    float m_pitch = 0.0f;
    float m_speed = 5.0f;
    float m_speedSens = 0.2f;
    float m_minSpeed = 0.01f;
    float m_maxSpeed = 20.0f;
    float m_sens = 0.1f;
};

//Orbit camera
class OrbitCamera : public ICamera {
public:
    void Update(Input* input, float deltaTime, const mathpp::vec3f&, bool suppressLook ) override;
    mathpp::vec3f GetPosition() const override;
    mathpp::mat4f GetViewMatrix() const override;
    bool IsDragging(Input* input) const override;

private:
    mathpp::vec3f m_eye;
    mathpp::vec3f m_target;
    mathpp::mat4f m_viewMatrix;
    mathpp::vec3f m_panOffset{};


    float m_yaw = 0.0f;
    float m_pitch = 0.0f;
    float m_distance = 5.0f;
    float m_sens = 0.1f;
    float m_zoom = 1.0f;
    float m_zoomSens = 0.2f;
    float m_minDistance = 0.01f;
    float m_maxDistance = 100.0f;
    float m_panSens = 0.001f;
};

// Owner / mode switcher
class Camera {
public:
    Camera();
    ~Camera();

    void Update(Input* input, float deltaTime, const mathpp::vec3f& target, bool suppressLook);
    mathpp::vec3f GetPosition() const;
    mathpp::mat4f GetViewMatrix() const;
    bool IsDragging(Input* input) const;

    void Switch();
    CameraType GetType() const { return em_mode; }

private:
    FreeCamera m_freeCam;
    OrbitCamera m_orbitCam;
    ICamera* p_active;
    CameraType em_mode;
};