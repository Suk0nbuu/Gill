#pragma once
#include "scene/scene.hpp"

struct LightBlock;
class TransformSystem;
class LightSystem {
public:
    void Init(TransformSystem* transformSystem);
    void Gather(Scene* scene,LightBlock& out);

private:
    TransformSystem* p_transformSystem;
};
