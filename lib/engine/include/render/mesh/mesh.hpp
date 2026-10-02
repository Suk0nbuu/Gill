#pragma once
#include <vector>


struct Vertex;
struct SkinnedVertex;


class Mesh {
public:
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    Mesh(const std::vector<SkinnedVertex>& vertices, const std::vector<unsigned int>& indices);
    void Draw() const;
    // delete copy . GL handles can't be safely duplicated
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    // allow move . transfer ownership of the handles, null out the source
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;
    bool IsSkinned() const;

    ~Mesh();
    private:
    unsigned int m_VAO = 0, m_VBO = 0, m_EBO = 0;
    unsigned int m_indexCount;
    bool m_skinned = false;
};