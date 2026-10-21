#include "editor/ui/shaderEditor.hpp"
#include "imgui.h"
#include "imnodes.h"
#include "shadergraph/nodeRegistry.hpp"
#include "editor/dialog/filePicker.hpp"
#include "editor/dialog/textureImport.hpp"
#include "editor/selector/selectionManager.hpp"
#include "core/project/projectPath.hpp"
#include "scene/scene.hpp"
#include <algorithm>
#include <filesystem>
#include <functional>
#include <unordered_set>

namespace {
    // attribute id = node id in the high bits, bit 7 = "is output", low 7 bits = socket index
    int  AttrId(int node, bool output, int socket) { return (node << 8) | (output ? 0x80 : 0) | socket; }
    int  AttrNode(int id)     { return id >> 8; }
    bool AttrIsOutput(int id) { return (id & 0x80) != 0; }
    int  AttrSocket(int id)   { return id & 0x7F; }

    ImU32 SocketColor(SocketType t) {                                // Blender's socket colors
        switch (t) {
            case SocketType::Float: return IM_COL32(161, 161, 161, 255);
            case SocketType::Vec3:  return IM_COL32( 99,  99, 199, 255);
            default:                return IM_COL32(199, 199,  41, 255);
        }
    }
    ImU32 CategoryColor(const std::string& c) {
        if (c == "Input" || c == "Output") return IM_COL32(150,  60,  60, 255);
        if (c == "Texture")                return IM_COL32(180, 100,  20, 255);
        if (c == "Vector")                 return IM_COL32( 80,  80, 160, 255);
        if (c == "Color")                  return IM_COL32(150, 150,  40, 255);
        return IM_COL32(40, 100, 120, 255);                          // Math and anything else
    }
    ImU32 Lighten(ImU32 c) {
        ImVec4 v = ImGui::ColorConvertU32ToFloat4(c);
        v.x = std::min(v.x * 1.3f, 1.0f); v.y = std::min(v.y * 1.3f, 1.0f); v.z = std::min(v.z * 1.3f, 1.0f);
        return ImGui::ColorConvertFloat4ToU32(v);
    }

    bool IsLinked(const ShaderGraph& g, int node, int socket) {
        for (const auto& l : g.links) if (l.toNode == node && l.toSocket == socket) return true;
        return false;
    }

    // does `node` read, directly or through other nodes, from `target`?
    bool DependsOn(const ShaderGraph& g, int node, int target, std::unordered_set<int>& seen) {
        if (node == target) return true;
        if (!seen.insert(node).second) return false;
        for (const auto& l : g.links)
            if (l.toNode == node && DependsOn(g, l.fromNode, target, seen)) return true;
        return false;
    }

    void DrawSocketValue(SocketValue& v, SocketType t) {             // the inline value of an unlinked input
        ImGui::SetNextItemWidth(90.0f);
        if (t == SocketType::Float) {
            if (auto* f = std::get_if<float>(&v)) ImGui::DragFloat("##v", f, 0.01f);
        } else if (auto* c = std::get_if<mathpp::vec3f>(&v)) {
            if (t == SocketType::Color) ImGui::ColorEdit3("##v", &c->x, ImGuiColorEditFlags_NoInputs);
            else                        ImGui::DragFloat3("##v", &c->x, 0.01f);
        }
    }

    bool ContainsNoCase(std::string hay, std::string needle) {
        auto lower = [](std::string& s) { std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); }); };
        lower(hay); lower(needle);
        return hay.find(needle) != std::string::npos;
    }
}

ShaderEditor::ShaderEditor(EditorContext ctx) : m_ctx(ctx) {
    p_nodesContext = ImNodes::CreateContext();
    ImNodes::StyleColorsDark();
}

ShaderEditor::~ShaderEditor() {
    ImNodes::DestroyContext(p_nodesContext);
}

Material* ShaderEditor::SelectedMaterial(AssetHandle& handle) {
    auto sel = m_ctx.p_selectionManager->GetActiveSelected();
    if (!sel) return nullptr;
    const auto* c = m_ctx.p_scene->TryGetComponent<comp::MaterialComponent>(*sel);
    if (!c) return nullptr;
    handle = c->materialID;
    return m_ctx.p_materialSystem->GetMaterial(c->materialID);
}

void ShaderEditor::DrawContents() {
    ImNodes::SetCurrentContext(p_nodesContext);
    AssetHandle handle{};
    Material* mat = SelectedMaterial(handle);
    if (!mat) {
        m_lastMaterial = {};
        ImGui::TextDisabled("Select an entity that has a material.");
        return;
    }
    if (!mat->graph) {
        m_lastMaterial = {};
        ImGui::TextDisabled("This material uses a builtin shader.");
        if (ImGui::Button("Create node graph")) {
            std::string err;
            m_error = m_ctx.p_materialSystem->CreateDefaultGraph(*mat, err) ? "" : err;
        }
        return;
    }

    ShaderGraph& g = *mat->graph;
    const NodeRegistry& reg = NodeRegistry::Get();

    const bool fresh = (handle != m_lastMaterial);        // a different graph than last frame
    m_lastMaterial = handle;
    if (fresh) {
        ImNodes::ClearNodeSelection();
        ImNodes::ClearLinkSelection();
        std::string err;
        m_error = m_ctx.p_materialSystem->CompileGraph(*mat, err) ? "" : err;
    }
    if (!m_error.empty()) ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", m_error.c_str());

    bool structureChanged = false;

    const bool focused =
    ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);



    const bool rightReleased =
        ImGui::IsMouseReleased(ImGuiMouseButton_Right);

    const bool shiftA =
        ImGui::GetIO().KeyShift &&
        ImGui::IsKeyPressed(ImGuiKey_A);

        // Shift+A, like Blender

    ImNodes::BeginNodeEditor();
    ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick);
    const bool hovered = ImNodes::IsEditorHovered();
    const bool openPopup = focused && hovered && (rightReleased || shiftA);

    for (GraphNode& n : g.nodes) {
        const NodeDef* def = reg.Find(n.type);
        if (!def) continue;
        if (fresh) ImNodes::SetNodeGridSpacePos(n.id, ImVec2(n.posX, n.posY));

        const ImU32 header = CategoryColor(def->category);
        ImNodes::PushColorStyle(ImNodesCol_TitleBar, header);
        ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, Lighten(header));
        ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, Lighten(header));
        ImNodes::BeginNode(n.id);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted(def->label.c_str());
        ImNodes::EndNodeTitleBar();

        for (size_t k = 0; k < def->out.size(); ++k) {                   // outputs on top, like Blender
            const ImU32 col = SocketColor(def->out[k].type);
            ImNodes::PushColorStyle(ImNodesCol_Pin, col);
            ImNodes::PushColorStyle(ImNodesCol_PinHovered, Lighten(col));
            ImNodes::BeginOutputAttribute(AttrId(n.id, true, static_cast<int>(k)), ImNodesPinShape_CircleFilled);
            ImGui::TextUnformatted(def->out[k].name.c_str());
            ImNodes::EndOutputAttribute();
            ImNodes::PopColorStyle();
            ImNodes::PopColorStyle();
        }

        if (def->usesImage) {
            ImGui::PushID(n.id);
            const std::filesystem::path& root = m_ctx.p_projectPaths->Root();
            if (root.empty()) {
                ImGui::TextDisabled("No project open");
            } else {
                const std::string label = n.image.path.empty() ? "None (click to choose)"
                                                               : std::filesystem::path(n.image.path).filename().string();
                if (ImGui::Button(label.c_str())) {
                    if (auto picked = FilePicker::PickImageFile()) {
                        std::string err;
                        const std::string rel = ImportTexture(*picked, root, err);
                        if (!rel.empty()) n.image = TextureRef{rel, m_ctx.p_materialSystem->GetOrLoadTexture(rel)};
                        // same sampler name either way, so no recompile is needed
                    }
                }
            }
            ImGui::PopID();
        }

        for (size_t k = 0; k < def->in.size(); ++k) {
            const SocketDef& sd = def->in[k];
            const int attr = AttrId(n.id, false, static_cast<int>(k));
            const ImU32 col = SocketColor(sd.type);
            ImNodes::PushColorStyle(ImNodesCol_Pin, col);
            ImNodes::PushColorStyle(ImNodesCol_PinHovered, Lighten(col));
            ImNodes::BeginInputAttribute(attr, ImNodesPinShape_CircleFilled);
            ImGui::TextUnformatted(sd.name.c_str());
            if (!IsLinked(g, n.id, static_cast<int>(k)) && k < n.inputValues.size()) {
                ImGui::SameLine();
                ImGui::PushID(attr);
                DrawSocketValue(n.inputValues[k], sd.type);              // changes uniforms only, no recompile
                ImGui::PopID();
            }
            ImNodes::EndInputAttribute();
            ImNodes::PopColorStyle();
            ImNodes::PopColorStyle();
        }

        ImNodes::EndNode();
        ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();
    }

    for (size_t i = 0; i < g.links.size(); ++i) {
        const GraphLink& l = g.links[i];
        const NodeDef* from = nullptr;
        for (const auto& n : g.nodes) if (n.id == l.fromNode) { from = reg.Find(n.type); break; }
        const ImU32 col = (from && l.fromSocket < static_cast<int>(from->out.size()))
                              ? SocketColor(from->out[l.fromSocket].type) : IM_COL32(200, 200, 200, 255);
        ImNodes::PushColorStyle(ImNodesCol_Link, col);
        ImNodes::Link(static_cast<int>(i), AttrId(l.fromNode, true, l.fromSocket), AttrId(l.toNode, false, l.toSocket));
        ImNodes::PopColorStyle();
    }
    ImNodes::PopAttributeFlag();

    if (openPopup && !ImGui::IsAnyItemHovered()) ImGui::OpenPopup("add_node");
    if (ImGui::BeginPopup("add_node")) {
        const ImVec2 clickPos = ImGui::GetMousePosOnOpeningCurrentPopup();
        auto addNode = [&](const NodeDef& d) {
            GraphNode n = MakeNode(d, g.nextId++);
            ImNodes::SetNodeScreenSpacePos(n.id, clickPos);
            g.nodes.push_back(std::move(n));                             // unlinked = unreachable, so no recompile yet
            ImGui::CloseCurrentPopup();
        };
        if (ImGui::IsWindowAppearing()) { m_search[0] = '\0'; ImGui::SetKeyboardFocusHere(); }
        ImGui::InputTextWithHint("##search", "Search...", m_search, sizeof(m_search));
        ImGui::Separator();

        if (m_search[0]) {
            for (const NodeDef& d : reg.All())
                if (!d.isOutput && ContainsNoCase(d.label, m_search) && ImGui::MenuItem(d.label.c_str())) addNode(d);
        } else {
            std::vector<std::string> cats;
            for (const NodeDef& d : reg.All())
                if (!d.isOutput && std::find(cats.begin(), cats.end(), d.category) == cats.end()) cats.push_back(d.category);
            for (const std::string& c : cats) {
                if (!ImGui::BeginMenu(c.c_str())) continue;
                for (const NodeDef& d : reg.All())
                    if (!d.isOutput && d.category == c && ImGui::MenuItem(d.label.c_str())) addNode(d);
                ImGui::EndMenu();
            }
        }
        ImGui::EndPopup();
    }
    ImNodes::EndNodeEditor();

    // keep the saved positions in sync with the canvas
    for (GraphNode& n : g.nodes) {
        if (!reg.Find(n.type)) continue;
        const ImVec2 p = ImNodes::GetNodeGridSpacePos(n.id);
        n.posX = p.x; n.posY = p.y;
    }

    int destroyed = 0;                                                    // 1) a link was dragged off its socket
    if (ImNodes::IsLinkDestroyed(&destroyed) && destroyed >= 0 && destroyed < static_cast<int>(g.links.size())) {
        g.links.erase(g.links.begin() + destroyed);
        structureChanged = true;
    }

    const bool wantDelete = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::GetIO().WantTextInput &&
                            (ImGui::IsKeyPressed(ImGuiKey_Delete) || ImGui::IsKeyPressed(ImGuiKey_X));
    if (wantDelete) {                                                     // 2) Delete or X removes the selection
        if (const int nl = ImNodes::NumSelectedLinks(); nl > 0) {
            std::vector<int> ids(nl);
            ImNodes::GetSelectedLinks(ids.data());
            std::sort(ids.begin(), ids.end(), std::greater<int>());
            for (int id : ids) if (id >= 0 && id < static_cast<int>(g.links.size())) g.links.erase(g.links.begin() + id);
            ImNodes::ClearLinkSelection();
            structureChanged = true;
        }
        if (const int nn = ImNodes::NumSelectedNodes(); nn > 0) {
            std::vector<int> ids(nn);
            ImNodes::GetSelectedNodes(ids.data());
            for (int id : ids) {
                if (id == g.outputNode) continue;                         // the output node can't be deleted
                g.nodes.erase(std::remove_if(g.nodes.begin(), g.nodes.end(), [&](const GraphNode& n) { return n.id == id; }), g.nodes.end());
                g.links.erase(std::remove_if(g.links.begin(), g.links.end(),
                              [&](const GraphLink& l) { return l.fromNode == id || l.toNode == id; }), g.links.end());
                structureChanged = true;
            }
            ImNodes::ClearNodeSelection();
        }
    }

    int a = 0, b = 0;                                                     // 3) a new link
    if (ImNodes::IsLinkCreated(&a, &b) && AttrIsOutput(a) != AttrIsOutput(b)) {
        if (!AttrIsOutput(a)) std::swap(a, b);                            // a = the output end, b = the input end
        const GraphLink l{AttrNode(a), AttrSocket(a), AttrNode(b), AttrSocket(b)};
        std::unordered_set<int> seen;
        if (l.fromNode != l.toNode && !DependsOn(g, l.fromNode, l.toNode, seen)) {     // refuse links that would make a cycle
            g.links.erase(std::remove_if(g.links.begin(), g.links.end(),
                          [&](const GraphLink& x) { return x.toNode == l.toNode && x.toSocket == l.toSocket; }), g.links.end());
            g.links.push_back(l);
            structureChanged = true;
        }
    }

    if (structureChanged) {
        std::string err;
        m_error = m_ctx.p_materialSystem->CompileGraph(*mat, err) ? "" : err;
    }
}