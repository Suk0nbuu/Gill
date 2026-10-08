#include "glad/gl.h"
#include "render/data/uniform.hpp"
UniformBuffer::UniformBuffer(size_t size, unsigned binding) {
    glGenBuffers(1, &m_id);
    glBindBuffer(GL_UNIFORM_BUFFER, m_id);
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_id);
}

UniformBuffer::~UniformBuffer() {
    glDeleteBuffers(1, &m_id);
}

void UniformBuffer::Update(const void *data, size_t size, size_t offset) {
    glBindBuffer(GL_UNIFORM_BUFFER, m_id);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
}
