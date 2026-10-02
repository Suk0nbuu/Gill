#include "render/mesh/mesh.hpp"
#include "render/data/vertex.hpp"
#include <glad/gl.h>
#include <GLFW/glfw3.h>

Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices) {
  m_indexCount = indices.size();
  glGenVertexArrays(1, &m_VAO);
  glBindVertexArray(m_VAO);
  glGenBuffers(1, &m_VBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

  glGenBuffers(1, &m_EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexCount * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex,position)));                          // position
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));         // normal
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex,uv)));         // uv
  glBindVertexArray(0);
  // m_skinned is false on default
}

void Mesh::Draw() const {
  glBindVertexArray(m_VAO);
  glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

Mesh::~Mesh() {
  glDeleteVertexArrays(1, &m_VAO);
  glDeleteBuffers(1, &m_VBO);
  glDeleteBuffers(1, &m_EBO);
}


Mesh::Mesh(Mesh&& other) noexcept
    : m_VAO(other.m_VAO), m_VBO(other.m_VBO), m_EBO(other.m_EBO), m_indexCount(other.m_indexCount) , m_skinned(other.m_skinned){
  other.m_VAO = other.m_VBO = other.m_EBO = 0; // so the moved-from destructor is a no-op
  other.m_indexCount = 0;
  other.m_skinned = false;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
  if (this != &other) {
    // clean up whatever this Mesh currently owns first
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);

    m_VAO = other.m_VAO; m_VBO = other.m_VBO; m_EBO = other.m_EBO;
    m_indexCount = other.m_indexCount;
    m_skinned = other.m_skinned;

    other.m_VAO = other.m_VBO = other.m_EBO = 0;
    other.m_indexCount = 0;
    other.m_skinned = false;
  }
  return *this;
}

Mesh::Mesh(const std::vector<SkinnedVertex> &vertices, const std::vector<unsigned int> &indices) {
  m_indexCount = indices.size();
  glGenVertexArrays(1, &m_VAO);
  glBindVertexArray(m_VAO);
  glGenBuffers(1, &m_VBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(SkinnedVertex), vertices.data(), GL_STATIC_DRAW);

  glGenBuffers(1, &m_EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexCount * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), reinterpret_cast<void*>(offsetof(SkinnedVertex,position)));  // position
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), reinterpret_cast<void*>(offsetof(SkinnedVertex,normal)));   // normal
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), reinterpret_cast<void*>(offsetof(SkinnedVertex,uv)));     // uv
  glEnableVertexAttribArray(3);
  glVertexAttribIPointer(3, 4, GL_INT, sizeof(SkinnedVertex), reinterpret_cast<void*>(offsetof(SkinnedVertex,boneIndices)));  // bone indices
  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), reinterpret_cast<void*>(offsetof(SkinnedVertex,boneWeights))); // bone weights
  glBindVertexArray(0);
  m_skinned = true;
}

bool Mesh::IsSkinned() const {
  return m_skinned;
}