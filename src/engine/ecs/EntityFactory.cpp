#include "EntityFactory.h"
#include "engine/ecs/EntityManager.h"
#include "engine/ecs/ComponentStorage.h"
#include "engine/ecs/Components.h"
#include "engine/scene/SceneData.h"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

EntityFactory::EntityFactory(EntityManager& em, ComponentStorage& cs)
    : entityManager(em), componentStorage(cs)
{
}

// ENTITY CREATION

Entity EntityFactory::createPlayer(float x, float y, float z){
    Entity player = entityManager.createEntity();
    // Transform
    TransformComponent transform;
    transform.x = x;
    transform.y = y;
    transform.z = z;
    componentStorage.AddComponent(player, transform);

    // Renderer
    Renderer r;
    r.color = BLUE; // Player is blue
    componentStorage.AddComponent(player, r);

    // Input
    componentStorage.AddComponent(player, PlayerInput{});

    // Stats
    componentStorage.AddComponent(player, Health{100, 100});
    componentStorage.AddComponent(player, Attack{});

    return player;
}

Entity EntityFactory::createFromGridCube(const GridCube& cubeData){
    Entity entity = entityManager.createEntity();

    // Convert grid position to world coordinates
    TransformComponent transform;
    transform.x = static_cast<float>(cubeData.position.x);
    transform.y = static_cast<float>(cubeData.position.y);
    transform.z = static_cast<float>(cubeData.position.z);
    componentStorage.AddComponent(entity, transform);

    Renderer r;
    // Set color or size based on the string type
    if (cubeData.type == "floor") {
        r.color = DARKGRAY;
    } else if (cubeData.type == "wall") {
        r.color = GRAY;
    } else if (cubeData.type == "door") {
        r.color = BROWN;
        
        // Add a SceneTransition component if it's a door
        SceneTransition transition;
        transition.targetScene = cubeData.targetScene;
        componentStorage.AddComponent(entity, transition);
    }
    componentStorage.AddComponent(entity, r);

    // Add collision if solid
    if (cubeData.solid) {
        Collider col;
        col.isTrigger = false;
        componentStorage.AddComponent(entity, col);
    }

    // Add trigger collision if it's a trigger
    if (cubeData.trigger) {
        Collider col;
        col.isTrigger = true;
        componentStorage.AddComponent(entity, col);
    }

    return entity;
}

Entity EntityFactory::createTestCube(float x, float y, float z){
    Entity entity = entityManager.createEntity();

    TransformComponent transform;
    transform.x = x;
    transform.y = y;
    transform.z = z;
    componentStorage.AddComponent(entity, transform);

    Renderer r;
    r.color = RED; // Default test cube color
    componentStorage.AddComponent(entity, r);

    return entity;
}

// ------------------------------------------------------------------
// JSON SERIALIZATION / DESERIALIZATION
// ------------------------------------------------------------------

json EntityFactory::serialize(Entity entity){
    json j;

    // 1. Explicitly save the Entity ID!
    j["id"] = entity.id; 

    // 2. Serialize the SpawnPoint component
    if (componentStorage.HasComponent<SpawnPoint>(entity)) {
        const auto& sp = componentStorage.GetComponent<SpawnPoint>(entity);
        j["SpawnPoint"] = {
            {"entityToSpawn", sp.entityToSpawn},
            {"hasSpawned", sp.hasSpawned}
        };
    }
    // 1. Serialize TransformComponent
    if (componentStorage.HasComponent<TransformComponent>(entity)) {
        const auto& t = componentStorage.GetComponent<TransformComponent>(entity);
        j["TransformComponent"] = {
            {"x", t.x}, {"y", t.y}, {"z", t.z},
            {"width", t.width}, {"height", t.height}, {"depth", t.depth}
        };
    }

    // 2. Serialize Renderer
    if (componentStorage.HasComponent<Renderer>(entity)) {
        const auto& r = componentStorage.GetComponent<Renderer>(entity);
        j["Renderer"] = {
            {"textureID", r.textureID},
            {"color", {r.color.r, r.color.g, r.color.b, r.color.a}}, // Raylib Color to array
            {"width", r.width}, {"height", r.height}, {"depth", r.depth},
            {"zIndex", r.zIndex}
        };
    }

    // 3. Serialize PlayerInput (Tag component, just needs to exist)
    if (componentStorage.HasComponent<PlayerInput>(entity)) {
        j["PlayerInput"] = true; 
    }

    // Add other components (Health, Collider, etc.) following the same pattern...

    return j;
}

Entity EntityFactory::deserialize(const json& j){
    Entity entity = entityManager.createEntity();

    // 1. Deserialize TransformComponent
    if (j.contains("TransformComponent")) {
        TransformComponent t;
        t.x = j["TransformComponent"].value("x", 0.0f);
        t.y = j["TransformComponent"].value("y", 0.0f);
        t.z = j["TransformComponent"].value("z", 0.0f);
        t.width = j["TransformComponent"].value("width", 32.0f);
        t.height = j["TransformComponent"].value("height", 32.0f);
        t.depth = j["TransformComponent"].value("depth", 32.0f);
        componentStorage.AddComponent(entity, t);
    }

    // 2. Deserialize Renderer
    if (j.contains("Renderer")) {
        Renderer r;
        r.textureID = j["Renderer"].value("textureID", "");
        
        if (j["Renderer"].contains("color")) {
            r.color.r = j["Renderer"]["color"][0];
            r.color.g = j["Renderer"]["color"][1];
            r.color.b = j["Renderer"]["color"][2];
            r.color.a = j["Renderer"]["color"][3];
        }

        r.width = j["Renderer"].value("width", 0.5f);
        r.height = j["Renderer"].value("height", 0.5f);
        r.depth = j["Renderer"].value("depth", 0.5f);
        r.zIndex = j["Renderer"].value("zIndex", 0);
        
        componentStorage.AddComponent(entity, r);
    }

    // 3. Deserialize PlayerInput
    if (j.contains("PlayerInput")) {
        componentStorage.AddComponent(entity, PlayerInput{});
    }

    if (j.contains("SpawnPoint")) {
        SpawnPoint sp;
        sp.entityToSpawn = j["SpawnPoint"].value("entityToSpawn", "");
        sp.hasSpawned = j["SpawnPoint"].value("hasSpawned", false);
        componentStorage.AddComponent(entity, sp);
    }

    // Add other components following the same pattern...

    return entity;
}

bool EntityFactory::saveEntitiesToFile(const std::vector<Entity>& entities, const std::string& filename)
{
    json jsonArray = json::array();

    for (Entity entity : entities) {
        json entityData = serialize(entity);
        if (!entityData.empty()) {
            jsonArray.push_back(entityData);
        }
    }

    // Open the file using the parameter!
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cout << "Error: Could not open file for saving -> " << filename << '\n';
        return false;
    }

    file << jsonArray.dump(4);
    file.close();

    std::cout << "Successfully saved " << jsonArray.size() << " entities to " << filename << '\n';
    return true;
}