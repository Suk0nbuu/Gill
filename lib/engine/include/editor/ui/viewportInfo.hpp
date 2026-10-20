#pragma once

struct ViewportInfo {
    float x = 0.0f, y = 0.0f;     // top-left of the image, in screen coordinates
    int width = 1, height = 1;    // the size the framebuffer should be next frame
    bool hovered = false;         // the mouse is over the 3D view
    bool focused = false;
    bool visible = false;         // some area is currently showing the 3D viewport
};