#include "SpawnSystem.h"
#include "engine/ecs/Components.h"

SpawnSystem::SpawnSystem(ComponentStorage& cs, EntityFactory& ef)
    : componentStorage(cs), entityFactory(ef) 
{
}

void SpawnSystem::Update() {
    // 1. Get ALL entities that have a SpawnPoint component
    auto& spawnTypes = componentStorage.GetComponents<SpawnType>();

    // 2. Iterate through them
    // Using structured binding (C++17) to get the Entity ID and the Component data
    for (auto& [entity, spawnType] : spawnTypes) {
        
        // 3. Check if it needs to spawn
        if (!spawnType.hasSpawned) {
            
            // 4. Look for a TransformComponent so we know WHERE to spawn it
            if (componentStorage.HasComponent<TransformComponent>(entity)) {
                auto& transform = componentStorage.GetComponent<TransformComponent>(entity);

                // 5. Spawn the correct entity based on the string ID
                if (spawnType.entityToSpawn == "player") {
                    entityFactory.createPlayer(transform.x, transform.y, transform.z);
                } 
                else if (spawnType.entityToSpawn == "test_cube") {
                    entityFactory.createTestCube(transform.x, transform.y, transform.z);
                }
                // Add your other entity types here (e.g., "enemy_goblin", "npc_merchant")

                // 6. Mark it as spawned so we don't spawn infinite copies!
                spawnType.hasSpawned = true;
            }
        }
    }
}