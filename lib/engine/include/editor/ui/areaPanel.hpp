#pragma once
#include "editor/ui/ui.hpp"
#include "editor/ui/viewportInfo.hpp"
#include "editor/ui/shaderEditor.hpp"
#include "render/framebuffer/framebuffer.hpp"
#include <memory>

enum class EditorType { Viewport3D, ShaderEditor };

class AreaPanel : public EditorPanel {
public:
    AreaPanel(EditorContext ctx, int id, EditorType type, Framebuffer* fb, ViewportInfo* info);
    void Draw() override;

private:
    void DrawHeader();
    void DrawViewport();

    EditorContext m_ctx;
    int m_id;
    EditorType m_type;
    Framebuffer* p_framebuffer;
    ViewportInfo* p_viewportInfo;
    std::unique_ptr<ShaderEditor> up_shaderEditor;     // created the first time this area shows the shader editor
};