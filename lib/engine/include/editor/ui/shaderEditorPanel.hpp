#pragma once
#include "editor/ui/ui.hpp"

struct ImNodesContext;

class ShaderEditorPanel : public EditorPanel {
public:
    explicit ShaderEditorPanel(EditorContext ctx);
    ~ShaderEditorPanel() override;
    ShaderEditorPanel(const ShaderEditorPanel&) = delete;
    ShaderEditorPanel& operator=(const ShaderEditorPanel&) = delete;
    void Draw() override;

private:
    Material* SelectedMaterial(AssetHandle& handle);

    EditorContext m_ctx;
    ImNodesContext* p_nodesContext = nullptr;
    AssetHandle m_lastMaterial;          // which material's graph was drawn last frame (positions are re-applied on change)
    std::string m_error;                 // last compile error, shown in red
    char m_search[64] = {};
};