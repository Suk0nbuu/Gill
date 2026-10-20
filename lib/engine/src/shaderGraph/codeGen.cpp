#include "shadergraph/codeGen.hpp"
#include <functional>
#include <unordered_map>

namespace {
    const char* GlslType(SocketType t) { return t == SocketType::Float ? "float" : "vec3"; }

    void Replace(std::string& s, const std::string& from, const std::string& to) {
        for (size_t p = 0; (p = s.find(from, p)) != std::string::npos; p += to.size())
            s.replace(p, from.size(), to);
    }

    // The same conversions Blender does on links
    std::string Convert(const std::string& e, SocketType from, SocketType to) {
        if (from == to) return e;
        if (from == SocketType::Float) return "vec3(" + e + ")";
        if (to == SocketType::Float)
            return from == SocketType::Color ? "dot(" + e + ", vec3(0.2126, 0.7152, 0.0722))"   // luminance
                                             : "dot(" + e + ", vec3(1.0 / 3.0))";                // average
        return e;                                                                                // Vec3 <-> Color
    }

    std::string Var(int node, size_t k) { return "n" + std::to_string(node) + "_" + std::to_string(k); }
}

CodegenResult GenerateFragment(const ShaderGraph& g, const NodeRegistry& reg) {
    CodegenResult R;
    auto fail = [&](std::string msg) { R.error = std::move(msg); return R; };

    auto findNode = [&](int id) -> const GraphNode* {
        for (const auto& n : g.nodes) if (n.id == id) return &n;
        return nullptr;
    };
    auto findLink = [&](int toNode, size_t toSocket) -> const GraphLink* {
        for (const auto& l : g.links) if (l.toNode == toNode && l.toSocket == (int)toSocket) return &l;
        return nullptr;
    };

    const GraphNode* out = findNode(g.outputNode);
    if (!out) return fail("graph has no output node");
    const NodeDef* outDef = reg.Find(out->type);
    if (!outDef || !outDef->isOutput) return fail("output node has the wrong type");

    // 1. Reachable nodes in dependency order (DFS from the output). Unreachable nodes are ignored.
    std::vector<const GraphNode*> order;
    std::unordered_map<int, int> state;                          // 1 = in progress, 2 = done
    std::string err;
    std::function<bool(const GraphNode*)> visit = [&](const GraphNode* n) -> bool {
        if (state[n->id] == 2) return true;
        if (state[n->id] == 1) { err = "graph contains a cycle"; return false; }
        state[n->id] = 1;
        const NodeDef* def = reg.Find(n->type);
        if (!def) { err = "unknown node type '" + n->type + "'"; return false; }
        for (size_t k = 0; k < def->in.size(); ++k) {
            const GraphLink* l = findLink(n->id, k);
            if (!l) continue;
            const GraphNode* src = findNode(l->fromNode);
            const NodeDef* sdef = src ? reg.Find(src->type) : nullptr;
            if (!sdef || l->fromSocket < 0 || l->fromSocket >= (int)sdef->out.size()) {
                err = "a link points at a missing node or socket"; return false;
            }
            if (!visit(src)) return false;
        }
        state[n->id] = 2;
        order.push_back(n);
        return true;
    };
    if (!visit(out)) return fail(err);

    // 2. Which nodes depend on the current light? Decided in dependency order.
    std::unordered_map<int, bool> perLight;
    for (const GraphNode* n : order) {
        const NodeDef* def = reg.Find(n->type);
        bool pl = def->perLight;
        for (size_t k = 0; k < def->in.size() && !pl; ++k)
            if (const GraphLink* l = findLink(n->id, k)) pl = perLight[l->fromNode];
        perLight[n->id] = pl;
    }
    if (const GraphLink* l = findLink(out->id, 0); l && perLight[l->fromNode])
        return fail("Base Color can't depend on per-light values (Light Direction / Light Radiance)");

    // 3. Emit
    std::string decls, once, loop;
    auto resolveInput = [&](const GraphNode* n, const NodeDef* def, size_t k) -> std::string {
        const SocketDef& sd = def->in[k];
        if (const GraphLink* l = findLink(n->id, k)) {
            const NodeDef* sdef = reg.Find(findNode(l->fromNode)->type);
            return Convert(Var(l->fromNode, l->fromSocket), sdef->out[l->fromSocket].type, sd.type);
        }
        const std::string u = "u_n" + std::to_string(n->id) + "_" + std::to_string(k);   // unlinked: a uniform
        decls += "uniform " + std::string(GlslType(sd.type)) + " " + u + ";\n";
        R.uniforms.push_back({u, sd.type, n->id, (int)k});
        return u;
    };

    for (const GraphNode* n : order) {
        const NodeDef* def = reg.Find(n->type);
        if (def->isOutput) continue;

        std::string code;
        for (size_t k = 0; k < def->out.size(); ++k)
            code += "    " + std::string(GlslType(def->out[k].type)) + " " + Var(n->id, k) + ";\n";

        std::string snippet = def->glsl;
        for (size_t k = 0; k < def->in.size(); ++k)
            Replace(snippet, "{in" + std::to_string(k) + "}", resolveInput(n, def, k));
        for (size_t k = 0; k < def->out.size(); ++k)
            Replace(snippet, "{out" + std::to_string(k) + "}", Var(n->id, k));
        if (def->usesImage) {
            const std::string tex = "tex_n" + std::to_string(n->id);
            decls += "uniform sampler2D " + tex + ";\nuniform int has_" + tex + " = 0;\n";
            R.textures.push_back({tex, n->id});
            Replace(snippet, "{tex}", tex);
        }
        (perLight[n->id] ? loop : once) += code + "    " + snippet + "\n";
    }

    const std::string base = resolveInput(out, outDef, 0);
    const std::string resp = resolveInput(out, outDef, 1);

    R.fragSource =
        "#version 330 core\n\n"
        "#include \"../core/camera.glsl\"\n"          // include base dir is a Shader-from-source question (next step)
        "#include \"../core/light.glsl\"\n\n"
        "in vec3 normals;\nin vec3 fragPos;\nin vec2 vUV;\n\n"
        + decls +
        "\nout vec4 fragColor;\n\nvoid main() {\n"
        + once +
        "    vec3 baseColor = " + base + ";\n"
        "    vec3 lighting = vec3(0.0);\n"
        "    for (int i = 0; i < lightCount; ++i) {\n"
        "        LightSample s = SampleLight(i, fragPos);\n"
        + loop +
        "        lighting += " + resp + ";\n"
        "    }\n"
        "    lighting = min(lighting, vec3(1.0));\n"
        "    vec3 color = pow(baseColor * lighting, vec3(1.0 / 2.2));\n"
        "    fragColor = vec4(color, 1.0);\n"
        "}\n";
    R.ok = true;
    return R;
}