#pragma once
#include "shadergraph/graph.hpp"

struct SocketDef { std::string name; SocketType type; SocketValue def; };

struct NodeDef {
    std::string type, label, category;
    std::vector<SocketDef> in, out;
    std::string glsl;               // placeholders: {in0} {in1}.. {out0} {out1}.. {tex}
    bool usesImage = false;         // node owns a texture (adds a sampler uniform and a has_ flag)
    bool perLight  = false;         // value changes per light (Light Direction, Light Radiance)
    bool isOutput  = false;
};

class NodeRegistry {
public:
    static const NodeRegistry& Get();
    const NodeDef* Find(const std::string& type) const;
    const std::vector<NodeDef>& All() const { return m_defs; }     // the Add menu is built from this
private:
    NodeRegistry();
    std::vector<NodeDef> m_defs;
};

GraphNode MakeNode(const NodeDef& def, int id);                    // inputValues filled from defaults