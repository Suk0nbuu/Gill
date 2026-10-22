#include "editor/ui/areaPanel.hpp"
#include "imgui.h"
#include <algorithm>
#include <cstdint>

namespace {
    const char* TypeName(EditorType t) {
        return t == EditorType::Viewport3D ? "3D Viewport" : "Shader Editor";
    }
}

AreaPanel::AreaPanel(EditorContext ctx, int id, EditorType type, Framebuffer* fb, ViewportInfo* info)
    : m_ctx(ctx), m_id(id), m_type(type), p_framebuffer(fb), p_viewportInfo(info) {}

void AreaPanel::Draw() {
    // the text before ### is the visible title, the part after it is the window's identity,
    // so docking and the saved layout survive switching the editor type
    const std::string title = std::string(TypeName(m_type)) + "###area" + std::to_string(m_id);

    const bool isViewport = (m_type == EditorType::Viewport3D);
    ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar;
    if (isViewport) flags |= ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    if (isViewport) ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    const bool open = ImGui::Begin(title.c_str(), nullptr, flags);
    if (isViewport) ImGui::PopStyleVar();

    if (open) {
        DrawHeader();
        switch (m_type) {
            case EditorType::Viewport3D:
                DrawViewport();
                break;
            case EditorType::ShaderEditor:
                if (!up_shaderEditor) up_shaderEditor = std::make_unique<ShaderEditor>(m_ctx);
                up_shaderEditor->DrawContents();
                break;
        }
    }
    ImGui::End();                                       // always, even when the window is collapsed
}

void AreaPanel::DrawHeader() {                          // Blender's editor-type dropdown
    if (!ImGui::BeginMenuBar()) return;
    ImGui::SetNextItemWidth(150.0f);
    if (ImGui::BeginCombo("##editorType", TypeName(m_type))) {
        for (EditorType t : {EditorType::Viewport3D, EditorType::ShaderEditor})
            if (ImGui::Selectable(TypeName(t), t == m_type)) m_type = t;
        ImGui::EndCombo();
    }
    if (m_type == EditorType::ShaderEditor) {
        ImGui::SameLine();
        ImGui::TextDisabled("Right-click or Shift+A: add node    X / Del: delete");
    }
    ImGui::EndMenuBar();
}

void AreaPanel::DrawViewport() {
    const ImVec2 avail = ImGui::GetContentRegionAvail();
    const int w = std::max(1, static_cast<int>(avail.x));
    const int h = std::max(1, static_cast<int>(avail.y));

    // flipped V: OpenGL textures start at the bottom-left, ImGui images at the top-left
    ImGui::Image(static_cast<ImTextureID>(static_cast<intptr_t>(p_framebuffer->ColorTexture())),
                 ImVec2(static_cast<float>(w), static_cast<float>(h)), ImVec2(0, 1), ImVec2(1, 0));

    const ImVec2 min = ImGui::GetItemRectMin();
    p_viewportInfo->x = min.x;
    p_viewportInfo->y = min.y;
    p_viewportInfo->width = w;                          // used to resize the framebuffer next frame
    p_viewportInfo->height = h;
    p_viewportInfo->hovered = ImGui::IsItemHovered();
    p_viewportInfo->focused = ImGui::IsWindowFocused();
    p_viewportInfo->visible = true;
}