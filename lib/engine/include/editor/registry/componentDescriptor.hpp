#pragma once
#include <string_view>
#include <vector>
#include "component/entity.hpp"
#include "core/engine/engineContext.hpp"

struct EditorContext;

struct ComponentDescriptor {
    const char* name;
    bool (*Has)(const EngineContext&, Entity);
    bool (*DrawInspector)(EditorContext&, Entity);
    void (*DrawAddMenu)(EditorContext&, Entity);
    void (*Remove)(const EngineContext&, Entity);      // nullptr = not removable
    const char* requires_ = nullptr;
};

class ComponentRegistry {
public:
    void Register(const ComponentDescriptor& d) { m_all.push_back(d); }
    const std::vector<ComponentDescriptor>& All() const { return m_all; }
    const ComponentDescriptor* Find(std::string_view name) const {
        for (const auto& d : m_all) if (name == d.name) return &d;
        return nullptr;
    }
private:
    std::vector<ComponentDescriptor> m_all;
};