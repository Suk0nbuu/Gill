#pragma once
#include "component/entity.hpp"
#include "component/light.hpp"
struct EngineContext;
Entity SpawnLight(EngineContext& ctx, comp::LightType type = comp::LightType::Point);