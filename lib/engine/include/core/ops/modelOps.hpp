#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include "component/entity.hpp"
struct EngineContext;
std::optional<Entity> SpawnModelMesh(EngineContext& ctx, const std::filesystem::path& src, std::string& err);


