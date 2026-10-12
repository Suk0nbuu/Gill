#pragma once


class Scene;
struct LightBlock;
class TransformSystem;
class LightSystem {
public:
    void Init(TransformSystem* transformSystem);
    void Gather(Scene* scene,LightBlock& out);

private:
    TransformSystem* p_transformSystem;
};
