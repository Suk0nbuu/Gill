
#pragma once
#include "component/entity.hpp"
#include "component/primitive.hpp"
#include "core/engine/engineContext.hpp"
#include "component/light.hpp"
#include <filesystem>
#include <optional>

struct PrimitiveData;

void DestroyEntityTree(const EngineContext& ctx, Entity root);
bool AddPrimitiveMesh(const EngineContext& ctx, Entity e, const comp::PrimitiveComponent& prim);
void RemoveMeshFromEntity(const EngineContext& ctx, Entity e);
bool AddMeshFromModel(EngineContext& ctx, Entity e, const std::filesystem::path& src, std::string& err);
void AddDefaultMaterial(EngineContext& ctx, Entity e);
Entity SpawnEmpty(EngineContext& ctx, std::string_view name = "");
std::optional<Entity> InstantiateModel(EngineContext&, const std::filesystem::path&, std::string&);
