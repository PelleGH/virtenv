#pragma once

#include "engine/ecs/Entity.h"
#include <string>
#include <nlohmann/json.hpp>

// Forward declarations
class EntityManager;
class ComponentStorage;
struct GridCube;

class EntityFactory
{
public:
    EntityFactory(EntityManager& em, ComponentStorage& cs);

    Entity createPlayer(float x, float y, float z, int skinChoice = 1);
    Entity createTestCube(float x, float y, float z, int skinChoice = 1);

    // --- NEW JSON SERIALIZATION ---
    // Converts an entity's components into a JSON object
    nlohmann::json serialize(Entity entity);

    // Creates an entity and attaches components based on a JSON object
    Entity deserialize(const nlohmann::json& j);

    // Converts a list of entities to JSON and saves them to a specific file
    bool saveEntitiesToFile(const std::vector<Entity>& entities, const std::string& filename = "src/engine/ecs/saved_entities.json");

private:
    EntityManager& entityManager;
    ComponentStorage& componentStorage;
};