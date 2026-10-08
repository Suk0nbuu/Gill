#include "editor/grid/grid.hpp"
#include "glad/gl.h"
#include "render/mesh/primitive.hpp"
#include "render/shader/shader.hpp"

void GridRenderer::Init(float extent) {
    up_gridMesh = CreateGridPlane(extent);
    m_ext = extent;
    up_gridShader = std::make_unique<Shader>(
        "asset/shader/gridShader/gridVert.glsl",
        "asset/shader/gridShader/gridFrag.glsl"
    );
}

void GridRenderer::Render(const mathpp::vec3f& camPos) {
    mathpp::mat4f model = mathpp::translate(mathpp::mat4f{}, {camPos.x, 0.0f, camPos.z});
    up_gridShader->Use();
    up_gridShader->setMat4f("model",model);
    up_gridShader->setFloat("cellSize", m_cellSize);
    up_gridShader->setFloat("extent", m_ext);



    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    up_gridMesh->Draw();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}