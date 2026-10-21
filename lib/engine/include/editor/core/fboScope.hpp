#pragma once
#include "glad/gl.h"

// Saves the current framebuffer binding and viewport, restores both on destruction.
// Use it in any pass that renders into its own FBO (picking, outlines, ...) so the
// caller's render target survives instead of falling back to the window (FBO 0).
struct FboScope {
    GLint fbo = 0;
    GLint vp[4] = {0, 0, 0, 0};
    FboScope()  { glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo); glGetIntegerv(GL_VIEWPORT, vp); }
    ~FboScope() { glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(fbo)); glViewport(vp[0], vp[1], vp[2], vp[3]); }
    FboScope(const FboScope&) = delete;
    FboScope& operator=(const FboScope&) = delete;
};
