#include "editor/picker/picker.hpp"
#include "glad/gl.h"
#include "editor/core/fboScope.hpp"
#include "mathpp.hpp"


void gill_editor::Picker::Init(unsigned int width, unsigned int height) {
    m_pickWidth = width;
    m_pickHeight = height;
    glGenFramebuffers(1,&m_pickFBO);
    glGenTextures(1,&m_pickTexture);
    glGenRenderbuffers(1,&m_pickDepthRenderbuffer);
}

void gill_editor::Picker::Resize(unsigned int width, unsigned int height) {
    width = std::max(width, 1u);
    height = std::max(height, 1u);
    if (width == m_pickWidth && height == m_pickHeight) return;
    m_pickWidth = width;
    m_pickHeight = height;

    FboScope scope;
    glBindFramebuffer(GL_FRAMEBUFFER,m_pickFBO);
    glBindTexture(GL_TEXTURE_2D,m_pickTexture);
    glTexImage2D(GL_TEXTURE_2D,0,GL_R32UI,m_pickWidth,m_pickHeight,0,GL_RED_INTEGER,GL_UNSIGNED_INT,nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,m_pickTexture,0);
    glBindRenderbuffer(GL_RENDERBUFFER,m_pickDepthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT,m_pickWidth,m_pickHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,m_pickDepthRenderbuffer);
}

void gill_editor::Picker::Run(const std::vector<Entry> &entry) {
    FboScope scope;
    glBindFramebuffer(GL_FRAMEBUFFER, m_pickFBO);
    glViewport(0, 0, m_pickWidth, m_pickHeight);
    Reset();
    glClearBufferuiv(GL_COLOR, 0,&m_clearValue);
    glClear(GL_DEPTH_BUFFER_BIT);
    for (auto& e : entry) {
        e.f_callable(e.type);
    }
}


uint32_t gill_editor::Picker::ReadAt(int x, int y) {

    FboScope scope;
    glBindFramebuffer(GL_FRAMEBUFFER, m_pickFBO);
    int flippedY = static_cast<int>(m_pickHeight) - 1 - y;  // glReadPixels origin is bottom-left

    GLuint pickedID = 0;
    glReadPixels(x, flippedY, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &pickedID);
}

void gill_editor::Picker::Reset() {
    glEnable(GL_DEPTH_TEST);

}
