#include "core/input/input.hpp"
#include <iostream>

void Input::Update() {
    double currentX,currentY;
    glfwGetCursorPos(p_window->GetWindow(),&currentX,&currentY);
    if (m_firstUpdate) {
        m_lastMousePos  = {static_cast<float>(currentX),static_cast<float>(currentY)};
        m_firstUpdate = false;
        m_mouseDelta = {0.0f,0.0f};
        m_scrollDelta = {0.0f, 0.0f};
        return;
    }

    m_mouseDelta.x = static_cast<float>(currentX) - m_lastMousePos.x;
    m_mouseDelta.y = static_cast<float>(currentY) - m_lastMousePos.y;
    m_lastMousePos = {static_cast<float>(currentX),static_cast<float>(currentY)};
    if (IsMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT) && !m_wasHeld) {
        m_wasHeld = true;
        mouseDown.Fire(static_cast<int>(currentX),static_cast<int>(currentY));
    }
    if (!IsMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT) && m_wasHeld) {
        m_wasHeld = false;
        mouseUp.Fire(static_cast<int>(currentX),static_cast<int>(currentY));
    }
    ar_prevKeys = ar_currKeys;
    for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; ++key) {
        int state = glfwGetKey(p_window->GetWindow(), key);
        if (state == GLFW_PRESS) {
            ar_currKeys[key] = true;
        } else if (state == GLFW_RELEASE) {
            ar_currKeys[key] = false;
        }
    }

}

void Input::GetCursorPos(mathpp::vec2f &pos) {
    double x,y;
    p_window->GetCursorPos(x,y);
    pos.x = static_cast<float>(x);
    pos.y = static_cast<float>(y);
}

bool Input::IsKeyDown(int Key) const {
    return glfwGetKey(p_window->GetWindow(),Key) == GLFW_PRESS;
}

bool Input::IsMouseButtonDown(int Button) const {
    return glfwGetMouseButton(p_window->GetWindow(),Button) == GLFW_PRESS;
}

mathpp::vec2f Input::GetMouseDelta() const {
    return m_mouseDelta;
}

void Input::GetScroll(mathpp::vec2f &scroll) {
    scroll = m_scrollDelta;
}

Input::Input(Window* window) {
    p_window = window;
    glfwSetWindowUserPointer(p_window->GetWindow(), this);
    glfwSetScrollCallback(p_window->GetWindow(), &Input::ScrollCallback);
}

void Input::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    auto* self = static_cast<Input*>(glfwGetWindowUserPointer(window));
    if (self) {
        self->m_scrollDelta.x += static_cast<float>(xoffset);
        self->m_scrollDelta.y += static_cast<float>(yoffset);
    }
}

Input::~Input() = default;

void Input::SetCursorMode(int mode) {
    CursorMode cursor = static_cast<CursorMode>(mode);
    if (cursor == CursorMode::Normal) {
        glfwSetInputMode(p_window->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    if (cursor == CursorMode::Hidden) {
        glfwSetInputMode(p_window->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }
    if (cursor == CursorMode::Disabled) {
        glfwSetInputMode(p_window->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

bool Input::IsKeyPressed(int key) const {
    return ar_currKeys[key] && !ar_prevKeys[key];
}

bool Input::IsKeyUp(int key) const {
    return !ar_currKeys[key];
}

bool Input::IsShiftHeld() const {
    return ar_currKeys[GLFW_KEY_LEFT_SHIFT] || ar_currKeys[GLFW_KEY_RIGHT_SHIFT];
}

void Input::ClearFrameState() {
    m_scrollDelta = {0.0f, 0.0f};
}


bool Input::WrapCursor(float minX, float minY, float maxX, float maxY, mathpp::vec2f& outCorrection) {
    double x, y;
    glfwGetCursorPos(p_window->GetWindow(), &x, &y);

    double newX = x, newY = y;
    bool wrapped = false;

    if (x < minX) { newX = maxX; wrapped = true; }
    else if (x > maxX) { newX = minX; wrapped = true; }
    if (y < minY) { newY = maxY; wrapped = true; }
    else if (y > maxY) { newY = minY; wrapped = true; }

    if (wrapped) {
        glfwSetCursorPos(p_window->GetWindow(), newX, newY);
        outCorrection = {static_cast<float>(newX - x), static_cast<float>(newY - y)};
        m_lastMousePos = {static_cast<float>(newX), static_cast<float>(newY)};
        return true;
    }
    outCorrection = {0.0f, 0.0f};
    return false;
}