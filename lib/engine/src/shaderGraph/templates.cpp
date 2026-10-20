#include "shadergraph/templates.hpp"
#include "shadergraph/nodeRegistry.hpp"
#include <cstdlib>

ShaderGraph MakeToonGraph() {
    const NodeRegistry& reg = NodeRegistry::Get();
    ShaderGraph g;
    auto add  = [&](const char* type) { g.nodes.push_back(MakeNode(*reg.Find(type), g.nextId++)); return g.nodes.back().id; };
    auto node = [&](int id) -> GraphNode& { for (auto& n : g.nodes) if (n.id == id) return n; std::abort(); };
    auto link = [&](int a, int ao, int b, int bi) { g.links.push_back({a, ao, b, bi}); };

    const int out  = add("output.toon");  g.outputNode = out;
    const int tex  = add("texture.image");
    const int tint = add("color.multiply");           // A = albedo tint (white), B = texture
    const int nrm  = add("input.normal");
    const int ldir = add("input.light_direction");
    const int lrad = add("input.light_radiance");
    const int dot  = add("vector.dot");
    const int ndl  = add("math.multiply_add");        // dot * 0.5 + 0.5  (half-lambert)
    const int edge = add("math.smoothstep");
    const int mix  = add("color.mix");
    const int resp = add("color.multiply");

    link(tex, 0, tint, 1);   link(tint, 0, out, 0);                   // Base Color
    link(nrm, 0, dot, 0);    link(ldir, 0, dot, 1);
    link(dot, 0, ndl, 0);    node(ndl).inputValues[1] = 0.5f;   node(ndl).inputValues[2] = 0.5f;
    link(ndl, 0, edge, 2);   node(edge).inputValues[0] = 0.48f; node(edge).inputValues[1] = 0.52f;
    link(edge, 0, mix, 0);   node(mix).inputValues[1] = mathpp::vec3f{0.55f, 0.55f, 0.7f};
                             node(mix).inputValues[2] = mathpp::vec3f{1.0f, 1.0f, 1.0f};
    link(mix, 0, resp, 0);   link(lrad, 0, resp, 1);
    link(resp, 0, out, 1);                                            // Light Response

    auto place = [&](int id, float x, float y) { node(id).posX = x; node(id).posY = y; };
    place(tex, 0, 0);       place(tint, 230, 0);
    place(nrm, 0, 170);     place(ldir, 0, 290);    place(lrad, 0, 430);
    place(dot, 230, 220);   place(ndl, 460, 220);   place(edge, 690, 220);   place(mix, 920, 220);
    place(resp, 1150, 320); place(out, 1380, 120);
    return g;
}