#include "core/system/light/light.hpp"
#include "render/data/lightBlock.hpp"
#include "mathpp.hpp"
#include "scene/scene.hpp"
#include "core/system/transform/transform.hpp"

void LightSystem::Init(TransformSystem* transformSystem) {
    p_transformSystem = transformSystem;
}

void LightSystem::Gather(Scene *scene, LightBlock &out) {
    out.count = 0;
    scene->ForEach<comp::LightComponent>([&](Entity e, const comp::LightComponent& l) {
        if (out.count >= (int)kMaxLights) return;
        const mathpp::mat4f w = p_transformSystem->GetWorldTransform(e);
        GpuLight& g = out.lights[out.count++];
        // col[c][r]: column 3 is translation, column 2 is local Z (forward is -Z)
        g.posType[0] = w.col[3][0]; g.posType[1] = w.col[3][1]; g.posType[2] = w.col[3][2];
        g.posType[3] = float(int(l.type));
        float fx = -w.col[2][0], fy = -w.col[2][1], fz = -w.col[2][2];
        float len = std::sqrt(fx*fx + fy*fy + fz*fz); if (len > 1e-6f) { fx/=len; fy/=len; fz/=len; }
        g.dirRange[0] = fx; g.dirRange[1] = fy; g.dirRange[2] = fz; g.dirRange[3] = l.range;
        g.colorInt[0] = l.color.x; g.colorInt[1] = l.color.y; g.colorInt[2] = l.color.z; g.colorInt[3] = l.intensity;
        float outer = l.spotSize * 0.5f * 3.14159265f / 180.0f;
        g.spot[0] = std::cos(outer * (1.0f - l.blend));   // inner
        g.spot[1] = std::cos(outer);
    });
}
