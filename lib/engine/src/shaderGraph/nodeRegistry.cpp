#include "shaderGraph/nodeRegistry.hpp"

const NodeRegistry& NodeRegistry::Get() { static NodeRegistry r; return r; }

const NodeDef* NodeRegistry::Find(const std::string& type) const {
    for (const auto& d : m_defs) if (d.type == type) return &d;
    return nullptr;
}

GraphNode MakeNode(const NodeDef& def, int id) {
    GraphNode n; n.id = id; n.type = def.type;
    for (const auto& s : def.in) n.inputValues.push_back(s.def);
    return n;
}

NodeRegistry::NodeRegistry() {
    const mathpp::vec3f white{1.0f, 1.0f, 1.0f}, black{0.0f, 0.0f, 0.0f}, grey{0.5f, 0.5f, 0.5f};
    using T = SocketType;

    auto add = [&](const char* type, const char* label, const char* cat,
                   std::vector<SocketDef> in, std::vector<SocketDef> out, const char* glsl) -> NodeDef& {
        NodeDef d; d.type = type; d.label = label; d.category = cat;
        d.in = std::move(in); d.out = std::move(out); d.glsl = glsl;
        m_defs.push_back(std::move(d));
        return m_defs.back();
    };

    add("input.normal", "Normal", "Input", {}, {{"Normal", T::Vec3, black}},
        "{out0} = normalize(normals);");
    add("input.light_direction", "Light Direction", "Input", {}, {{"Direction", T::Vec3, black}},
        "{out0} = s.L;").perLight = true;
    add("input.light_radiance", "Light Radiance", "Input", {}, {{"Radiance", T::Color, black}},
        "{out0} = s.radiance;").perLight = true;

    add("texture.image", "Image Texture", "Texture", {},
        {{"Color", T::Color, white}, {"Alpha", T::Float, 1.0f}},
        "{out0} = (has_{tex} != 0) ? pow(texture({tex}, vUV).rgb, vec3(2.2)) : vec3(1.0);\n"
        "    {out1} = (has_{tex} != 0) ? texture({tex}, vUV).a : 1.0;").usesImage = true;

    add("vector.dot", "Dot Product", "Vector",
        {{"A", T::Vec3, black}, {"B", T::Vec3, black}}, {{"Value", T::Float, 0.0f}},
        "{out0} = dot({in0}, {in1});");
    add("math.multiply_add", "Multiply Add", "Math",
        {{"A", T::Float, 1.0f}, {"B", T::Float, 1.0f}, {"C", T::Float, 0.0f}}, {{"Value", T::Float, 0.0f}},
        "{out0} = {in0} * {in1} + {in2};");
    add("math.smoothstep", "Smooth Step", "Math",
        {{"Min", T::Float, 0.0f}, {"Max", T::Float, 1.0f}, {"Value", T::Float, 0.5f}}, {{"Value", T::Float, 0.0f}},
        "{out0} = smoothstep({in0}, max({in1}, {in0} + 1e-4), {in2});");   // edge0 == edge1 is undefined in GLSL

    add("color.mix", "Mix", "Color",
        {{"Fac", T::Float, 0.5f}, {"A", T::Color, grey}, {"B", T::Color, white}}, {{"Color", T::Color, white}},
        "{out0} = mix({in1}, {in2}, clamp({in0}, 0.0, 1.0));");
    add("color.multiply", "Multiply", "Color",
        {{"A", T::Color, white}, {"B", T::Color, white}}, {{"Color", T::Color, white}},
        "{out0} = {in0} * {in1};");

    add("output.toon", "Toon Output", "Output",
        {{"Base Color", T::Color, white}, {"Light Response", T::Color, white}}, {}, "").isOutput = true;
}