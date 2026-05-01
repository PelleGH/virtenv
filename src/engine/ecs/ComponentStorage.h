#pragma once
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <cassert>
#include "Entity.h"
#include "ComponentArray.h"

//the Component master manager. It owns all the ComponentArrays (the drawers) and handles all the routing so you don't have to


class ComponentStorage {
private:
    // The Master Cabinet: Maps a component's C++ Type ID to its specific array (drawer)
    std::unordered_map<std::type_index, std::shared_ptr<IComponentArray>> componentArrays;

    // Internal helper: Finds the drawer and safely casts it to the exact type you need
    template <typename T>
    std::unique_ptr<ComponentArray<T>> GetComponentArray() {
        std::type_index typeName = typeid(T);
        assert(componentArrays.find(typeName) != componentArrays.end() && "Component not registered before use.");
        
        return std::static_pointer_cast<ComponentArray<T>>(componentArrays[typeName]);
    }

public:
    // 1. SETUP: Tell the engine this component exists (Call once at startup)
    template <typename T>
    void RegisterComponent() {
        std::type_index typeName = typeid(T);
        assert(componentArrays.find(typeName) == componentArrays.end() && "Registering component type more than once.");
        
        componentArrays[typeName] = std::make_shared<ComponentArray<T>>();
    }

    // 2. ATTACH: Give an entity some data
    template <typename T>
    void AddComponent(Entity entity, T component) {
        GetComponentArray<T>()->AddComponent(entity, component);
    }

    // 3. RETRIEVE: Get a specific entity's data so you can modify it
    template <typename T>
    T& GetComponent(Entity entity) {
        return GetComponentArray<T>()->GetComponent(entity);
    }

    // 4. CHECK: Ask if an entity has this data
    template <typename T>
    bool HasComponent(Entity entity) {
        return GetComponentArray<T>()->HasComponent(entity);
    }

    // 5. REMOVE: Strip a component off an entity
    template <typename T>
    void RemoveComponent(Entity entity) {
        GetComponentArray<T>()->RemoveComponent(entity);
    }

    // 6. ITERATE: Get the ENTIRE drawer (Crucial for Systems in your Game Loop)
    template <typename T>
    std::unordered_map<Entity, T>& GetComponents() {
        return GetComponentArray<T>()->GetComponents();
    }

    // 7. CLEANUP: Delete an entity's data from EVERY array
    void EntityDestroyed(Entity entity) {
        for (auto const& pair : componentArrays) {
            auto const& componentArray = pair.second;
            componentArray->EntityDestroyed(entity);
        }
    }
};