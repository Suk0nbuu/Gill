#include "core/ops/modelOps.hpp"
#include "core/ops/entityOps.hpp"

std::optional<Entity> SpawnModelMesh(EngineContext& ctx, const std::filesystem::path& src, std::string& err) {
    const Entity e = SpawnEmpty(ctx, src.stem().string());
    if (!AddMeshFromModel(ctx, e, src, err)) { DestroyEntityTree(ctx, e); return std::nullopt; }
    AddDefaultMaterial(ctx, e);
    return e;
}