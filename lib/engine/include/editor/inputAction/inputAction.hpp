#pragma once
#include <unordered_map>
#include <GLFW/glfw3.h>


enum class EditorAction {
    ToggleViewportMode,
    Translate,
    Rotate,
    Scale,
    AxisX,
    AxisY,
    AxisZ,
    ConfirmTransform,
    CancelTransform
};

class Input;


class EditorInputMap {
public:
    EditorInputMap(Input* input) : p_input(input) {};
    bool IsActionPressed(EditorAction action);


private:
    Input* p_input;
    std::unordered_map<EditorAction, int> um_keybinds = {
        { EditorAction::ToggleViewportMode, GLFW_KEY_TAB },
        { EditorAction::Translate, GLFW_KEY_G },
        { EditorAction::Rotate, GLFW_KEY_R },
        { EditorAction::Scale, GLFW_KEY_S },
        { EditorAction::AxisX, GLFW_KEY_X },
        { EditorAction::AxisY, GLFW_KEY_Y },
        { EditorAction::AxisZ, GLFW_KEY_Z },
        { EditorAction::ConfirmTransform, GLFW_KEY_ENTER },
        { EditorAction::CancelTransform, GLFW_KEY_ESCAPE },
    };
};