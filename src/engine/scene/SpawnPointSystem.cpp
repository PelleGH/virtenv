#include "SpawnPointSystem.h"
#include "engine/ecs/Components.h"
#include <iostream>

void SpawnPointSystem::processSpawnPoints(Scene& scene, EntityFactory& factory)
{
    ComponentStorage& components = scene.getComponentStorage();
    
    // Get all entities that have a SpawnPoint component
    auto& spawnPoints = components.GetComponents<SpawnPoint>();

    for (auto& [entity, spawnData] : spawnPoints)
    {
        // Skip if it already spawned its target
        if (spawnData.hasSpawned) continue;

        // Ensure the spawn point has a location!
        if (!components.HasComponent<TransformComponent>(entity)) {
            std::cout << "Warning: SpawnPoint missing TransformComponent!\n";
            continue;
        }

        TransformComponent& tf = components.GetComponent<TransformComponent>(entity);

        // Spawn logic based on the requested string
        if (spawnData.entityToSpawn == "player") {
            // Ask the factory to create the player exactly at the spawn point's coordinates
            Entity newPlayer = factory.createPlayer(tf.x, tf.y, tf.z);

            scene.addEntityToScene(newPlayer);
            
            std::cout << "Spawned Player at " << tf.x << ", " << tf.y << ", " << tf.z << '\n';
        }
        else if (spawnData.entityToSpawn == "enemy_test") {
            Entity newEnemy = factory.createTestCube(tf.x, tf.y, tf.z);

            scene.addEntityToScene(newEnemy);
        }
        
        // Mark as spawned so we don't accidentally spawn it again next frame
        spawnData.hasSpawned = true; 
    }
}

void SpawnPointSystem::saveSpawnData(Scene& scene, EntityFactory& factory, const std::string& filename)
{
    ComponentStorage& components = scene.getComponentStorage();
    auto& spawnPoints = components.GetComponents<SpawnPoint>();

    // 1. Create a temporary list to hold JUST the spawn point entities
    std::vector<Entity> spawnPointEntities;

    // 2. Loop through and grab them
    for (auto& [entity, spawnData] : spawnPoints)
    {
        spawnPointEntities.push_back(entity);
    }

    // 3. Send ONLY that list to the factory!
    factory.saveEntitiesToFile(spawnPointEntities, filename);
}