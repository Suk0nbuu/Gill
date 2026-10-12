#pragma once

struct ComponentDescriptor {
    const char* name;
    bool (*Has)(const EngineContext&, Entity);
    bool (*DrawInspector)(EditorContext&, Entity);          // returns true if anything was edited
    void (*DrawAddMenu)(EditorContext&, Entity);            // draws its own menu item or submenu, calls the engine op
    void (*Remove)(const EngineContext&, Entity);           // nullptr = not removable
    const char* requires_ = nullptr;                        // name of another component that must exist first
};

class ComponentRegistry {
public:
    void Register(const ComponentDescriptor& d) { m_all.push_back(d); }   // registration order = display order
    const std::vector<ComponentDescriptor>& All() const { return m_all; }
    const ComponentDescriptor* Find(std::string_view name) const;
private:
    std::vector<ComponentDescriptor> m_all;
};