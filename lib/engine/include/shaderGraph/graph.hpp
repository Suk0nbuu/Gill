#pragma once
#include <variant>
#include <vector>
#include <string>
#include "mathpp.hpp"
#include "component/textureRef.hpp"

enum class SocketType { Float, Vec3, Color };           // Vec3 and Color are both GLSL vec3
using SocketValue = std::variant<float, mathpp::vec3f>;

struct GraphNode {
    int id = 0;
    std::string type;                                    // key into NodeRegistry
    float posX = 0.0f, posY = 0.0f;                      // editor position (used by the UI later)
    std::vector<SocketValue> inputValues;                // one per input socket, used when unlinked
    TextureRef image;                                    // only for nodes with usesImage
};

struct GraphLink { int fromNode, fromSocket, toNode, toSocket; };

struct ShaderGraph {
    std::vector<GraphNode> nodes;
    std::vector<GraphLink> links;
    int outputNode = -1;
    int nextId = 1;
    const GraphNode* Find(int id) const {
        for (const auto& n : nodes) if (n.id == id) return &n;
        return nullptr;
    }
};

