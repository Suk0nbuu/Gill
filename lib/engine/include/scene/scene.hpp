#pragma once
#include <vector>
#include "component/entity.hpp"
#include "component/mesh.hpp"
#include "component/sunlight.hpp"
#include <functional>
#include "core/set/sparseset.hpp"
#include "core/system/asset/asset.hpp"
#include "component/material.hpp"
#include <optional>
#include "render/mesh/mesh.hpp"
#include "component/armature.hpp"
#include <unordered_set>

class Scene {
public:

    ~Scene();


    Entity CreateEntity();
    void DestroyEntity(Entity entity);


    template <typename T>
    void InsertComponent(Entity entity,const T& component);

    template <typename T>
    void RemoveComponent(Entity entity);

    template <typename T>
    void ForEach(std::function<void(Entity entity, const T& component)> func) const;

    template <typename T>
    void ForEach(std::function<void(Entity entity, T& component)> func);


    template<typename T>
   const T* TryGetComponent(Entity entity) const;





    template<typename T>
T* TryGetComponent(Entity entity);

    template <typename T>
    T& GetComponent(Entity entity);

    template<typename T>
    const T& GetComponent(Entity entity) const;







    AssetHandle LoadMesh(Mesh mesh) { return m_meshManager.Load(std::move(mesh)); }













    std::vector<Entity> GetLivingEntities() const;

    uint32_t GetEntityCount();



    private:
    EntityManager m_entityManager;
    AssetManager<Mesh> m_meshManager;
    SparseSet<comp::MeshComponent> m_meshes;
    SparseSet<comp::SunlightComponent> m_sunlights;
    SparseSet<comp::MaterialComponent> m_materials;
    SparseSet<comp::ArmatureComponent> m_armatures;
};




    template<>
    inline comp::SunlightComponent& Scene::GetComponent<comp::SunlightComponent>(Entity entity) {
        return m_sunlights.Get(entity);
    }
    template<>
    inline const comp::SunlightComponent& Scene::GetComponent<comp::SunlightComponent>(Entity entity) const {
        return m_sunlights.Get(entity);
    }
    template<>
    inline comp::MeshComponent& Scene::GetComponent<comp::MeshComponent>(Entity entity) {
        return m_meshes.Get(entity);
    }
    template<>
    inline const comp::MeshComponent& Scene::GetComponent<comp::MeshComponent>(Entity entity) const {
        return m_meshes.Get(entity);
    }
    template<>
    inline comp::MaterialComponent& Scene::GetComponent<comp::MaterialComponent>(Entity entity) {
        return m_materials.Get(entity);
    }
    template<>
    inline const comp::MaterialComponent& Scene::GetComponent<comp::MaterialComponent>(Entity entity) const {
        return m_materials.Get(entity);
    }

    template<>
    inline comp::MeshComponent* Scene::TryGetComponent<comp::MeshComponent>(Entity entity) {
        if (m_meshes.Has(entity)) {
            return &m_meshes.Get(entity);
        }
        return nullptr;
    }
    template<>
    inline const comp::MeshComponent* Scene::TryGetComponent<comp::MeshComponent>(Entity entity) const {
        if (m_meshes.Has(entity)) {
            return &m_meshes.Get(entity);
        }
        return nullptr;
    }
    template<>
    inline comp::SunlightComponent* Scene::TryGetComponent<comp::SunlightComponent>(Entity entity) {
        if (m_sunlights.Has(entity)) {
            return &m_sunlights.Get(entity);
        }
        return nullptr;
    }
    template<>
    inline const comp::SunlightComponent* Scene::TryGetComponent<comp::SunlightComponent>(Entity entity) const {
        if (m_sunlights.Has(entity)) {
            return &m_sunlights.Get(entity);
        }
        return nullptr;
    }
    template<>
    inline const comp::MaterialComponent* Scene::TryGetComponent<comp::MaterialComponent>(Entity entity) const {
        if (m_materials.Has(entity)) {
            return &m_materials.Get(entity);
        }
        return nullptr;
    }

    template<>
    inline comp::MaterialComponent*  Scene::TryGetComponent<comp::MaterialComponent>(Entity entity) {
        if (m_materials.Has(entity)) {
            return &m_materials.Get(entity);
        }
        return nullptr;
    }
    template<>
    inline comp::ArmatureComponent* Scene::TryGetComponent<comp::ArmatureComponent>(Entity entity) {
        if (m_armatures.Has(entity)) {
            return &m_armatures.Get(entity);
        }
        return nullptr;
    }
    template<>
    inline const comp::ArmatureComponent* Scene::TryGetComponent<comp::ArmatureComponent>(Entity entity) const {
        if (m_armatures.Has(entity)) {
            return &m_armatures.Get(entity);
        }
        return nullptr;
    }
    template<>
    inline void Scene::InsertComponent<comp::MeshComponent>(Entity entity,const comp::MeshComponent& component) {
        m_meshes.Insert(entity, component);
    }
    template<>
    inline void Scene::InsertComponent<comp::MaterialComponent>(Entity entity,const comp::MaterialComponent& component) {
        m_materials.Insert(entity,component);
    }
    template<>
    inline void Scene::InsertComponent<comp::ArmatureComponent>(Entity entity, const comp::ArmatureComponent& component) {
        m_armatures.Insert(entity,component);
    }
    template<>
    inline void Scene::RemoveComponent<comp::ArmatureComponent>(Entity entity) {
        m_armatures.Remove(entity);
    }

    template<>
    inline void Scene::RemoveComponent<comp::MeshComponent>(Entity entity) {
        m_meshes.Remove(entity);
    }
    template<>
    inline void Scene::InsertComponent<comp::SunlightComponent>(Entity entity,const comp::SunlightComponent& component) {
        m_sunlights.Insert(entity, component);
    }
    template<>
    inline void Scene::RemoveComponent<comp::SunlightComponent>(Entity entity) {
        m_sunlights.Remove(entity);
    }
    template<>
    inline void Scene::RemoveComponent<comp::MaterialComponent>(Entity entity) {
        m_materials.Remove(entity);
    }

    template<>
    inline void Scene::ForEach<comp::MeshComponent>(std::function<void(Entity entity,const  comp::MeshComponent& component)> func) const{
        for (size_t it = 0; it<m_meshes.Size();it++) {
            func(m_meshes.GetEntity(it), m_meshes[it]);
        }
    }

    template<>
    inline void Scene::ForEach<comp::SunlightComponent>(std::function<void(Entity entity, const comp::SunlightComponent& component)> func) const {
        for (size_t it = 0; it<m_sunlights.Size();it++) {
            func(m_sunlights.GetEntity(it), m_sunlights[it]);
        }
    }
    template<>
    inline void Scene::ForEach<comp::MaterialComponent>(std::function<void(Entity entity, const comp::MaterialComponent& component)> func) const {
        for (size_t it = 0; it<m_materials.Size();it++) {
            func(m_materials.GetEntity(it), m_materials[it]);
        }
    }

