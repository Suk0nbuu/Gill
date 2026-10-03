#pragma once
#include <filesystem>
#include <optional>
#include <string>
#include "project/project.hpp"
#include "project/projectList.hpp"

class Window;

// Godot-style project manager. Runs its own ImGui context *before* Engine/Editor exist,
// then tears it down so UIManager can create the real one.
class Launcher {
public:
    Launcher();

    // Blocks until a project is chosen (returned) or the window is closed (nullopt).
    std::optional<Project> Run(Window& window);

private:
    void Draw();
    void DrawToolbar();
    void DrawProjectTable();
    void DrawNewProjectPopup();
    void DrawDeletePopup();
    void DrawErrorPopup();

    void OpenEntry(size_t index);
    void ImportProject();
    void ShowError(const std::string& message);
    bool HasSelection() const;
    bool SelectionUsable() const;   // selected and not missing on disk

    std::optional<std::filesystem::path> PickFolder(const std::filesystem::path& startIn);
    std::filesystem::path DefaultLocation() const;

    ProjectList m_list;
    std::optional<Project> m_result;
    int m_selected = -1;

    char m_nameBuf[64] = "NewGameProject";
    char m_locationBuf[512] = {};
    std::string m_createError;

    std::string m_error;
    bool m_showError = false;
};