#pragma once
#include "shadergraph/nodeRegistry.hpp"

struct UniformBinding { std::string name; SocketType type; int nodeId; int socketIndex; };
struct TextureBinding { std::string name; int nodeId; };       // sampler = name, flag = "has_" + name

struct CodegenResult {
    bool ok = false;
    std::string error;
    std::string fragSource;
    std::vector<UniformBinding> uniforms;     // values live in the graph, uploaded every draw
    std::vector<TextureBinding> textures;
};

CodegenResult GenerateFragment(const ShaderGraph& g, const NodeRegistry& reg);