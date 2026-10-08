#pragma once
#include "mathpp.hpp"
#include <memory>

class Mesh;
class Shader;

class GridRenderer {
public:
    GridRenderer() = default;
    void Init(float extent = 100.0f); // builds the quad mesh + compiles grid shader once
    void Render(const mathpp::vec3f& camPos);

private:
    std::unique_ptr<Mesh> up_gridMesh;
    std::unique_ptr<Shader> up_gridShader;
    float m_cellSize = 1.0f;
    float m_ext;
};