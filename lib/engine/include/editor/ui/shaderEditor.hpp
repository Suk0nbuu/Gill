#pragma once
#include "editor/ui/ui.hpp"

struct ImNodesContext;

class ShaderEditor  {
public:
    explicit ShaderEditor(EditorContext ctx);
    ~ShaderEditor();
    ShaderEditor(const ShaderEditor&) = delete;
    ShaderEditor& operator=(const ShaderEditor&) = delete;
    void DrawContents();

private:
    Material* SelectedMaterial(AssetHandle& handle);

    EditorContext m_ctx;
    ImNodesContext* p_nodesContext = nullptr;
    AssetHandle m_lastMaterial;          // which material's graph was drawn last frame (positions are re-applied on change)
    std::string m_error;                 // last compile error, shown in red
    char m_search[64] = {};
};