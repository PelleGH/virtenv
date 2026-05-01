#pragma once
#include <unordered_map>
#include <cassert>
#include "Entity.h"


// We need this base class so the master ComponentStorage can store a list of completely different array types (e.g., ComponentArray<Transform> and ComponentArray<Health>) inside a single collection.
class IComponentArray {
public:
    virtual ~IComponentArray() = default;
    virtual void EntityDestroyed(Entity entity) = 0;
};



// THE TEMPLATE ARRAY

template <typename T>
class ComponentArray : public IComponentArray {
private:
    // The actual data storage. Maps an Entity ID to its component data.
    std::unordered_map<Entity, T> components;

public:
    void AddComponent(Entity entity, T component) {
        // Prevent adding a component an entity already has
        assert(components.find(entity) == components.end() && "Component added to same entity more than once.");
        components[entity] = component;
    }

    void RemoveComponent(Entity entity) {
        assert(components.find(entity) != components.end() && "Removing non-existent component.");
        components.erase(entity);
    }

    T& GetComponent(Entity entity) {
        assert(components.find(entity) != components.end() && "Retrieving non-existent component.");
        return components[entity];
    }

    bool HasComponent(Entity entity) const {
        return components.find(entity) != components.end();
    }

    //so game loops can actually iterate through all the components of this type every frame.
    std::unordered_map<Entity, T>& GetComponents() {
        return components;
    }

    // Automatically cleans up the memory if an entity dies. The master ComponentStorage will call this on every array.
    void EntityDestroyed(Entity entity) override {
        if (HasComponent(entity)) {
            RemoveComponent(entity);
        }
    }
};