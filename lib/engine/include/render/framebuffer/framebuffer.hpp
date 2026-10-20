#pragma once

class Framebuffer {
public:
    Framebuffer() = default;                       // creates nothing until Resize, so no GL context needed
    ~Framebuffer();
    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    void Resize(int width, int height);            // recreates the attachments, no-op if the size is unchanged
    void Bind() const;                             // binds this framebuffer and sets glViewport to its size
    static void Unbind(int windowWidth, int windowHeight);
    unsigned int ColorTexture() const { return m_color; }
    int Width() const  { return m_width; }
    int Height() const { return m_height; }

private:
    void Destroy();
    unsigned int m_fbo = 0, m_color = 0, m_depth = 0;
    int m_width = 0, m_height = 0;
};