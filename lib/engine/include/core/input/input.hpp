#pragma once
#include "mathpp.hpp"
#include <array>
#include  "core/window/window.hpp"
#include "core/component/event/event.hpp"

enum class CursorMode {
    Normal = 0,
    Hidden = 1,
    Disabled = 2
};

class Input {
public:
    Input(Window* window);
    void Update();
    bool IsKeyDown( int Key) const;
    bool IsKeyPressed( int key) const;
    bool IsKeyUp( int key) const;
    bool IsMouseButtonDown( int Button) const;
    void GetScroll(mathpp::vec2f& scroll);
    void SetCursorMode(int mode);
    void GetCursorPos(mathpp::vec2f& pos);
    bool IsShiftHeld() const;
    ~Input();
    mathpp::vec2f GetMouseDelta() const;
    void ClearFrameState();
    EventDelegate<int,int> mouseUp;
    EventDelegate<int,int> mouseDown;
private:
    mathpp::vec2f m_mouseDelta;
    mathpp::vec2f m_lastMousePos;
    mathpp::vec2f m_scrollDelta{};
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    bool m_firstUpdate = true;
    bool m_wasHeld = false;
    Window* p_window;
    static constexpr int MaxKeys = GLFW_KEY_LAST + 1;
    std::array<bool,MaxKeys> ar_currKeys = {};
    std::array<bool,MaxKeys> ar_prevKeys = {};
};