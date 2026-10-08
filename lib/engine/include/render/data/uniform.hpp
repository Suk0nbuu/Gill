#pragma  once

class UniformBuffer {
public:
    UniformBuffer(size_t size, unsigned binding);
    ~UniformBuffer();
    UniformBuffer(const UniformBuffer&) = delete;
    UniformBuffer& operator=(const UniformBuffer&) = delete;
    void Update(const void* data, size_t size, size_t offset = 0);
private:
    unsigned m_id = 0;
};