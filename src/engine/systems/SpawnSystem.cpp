#include "SpawnSystem.h"
#include "engine/ecs/Components.h"
#include "engine/scene/Scene.h"

SpawnSystem::SpawnSystem(ComponentStorage& cs, EntityFactory& ef)
    : componentStorage(cs), entityFactory(ef) 
{
}

void SpawnSystem::Update(Scene* currentScene) {
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
                bool successfullyCreatedEntity = false;
                Entity newEntity;
                // 5. Spawn the correct entity based on the string ID
                if (spawnType.entityToSpawn == "player") {
                    newEntity = entityFactory.createPlayer(transform.x, transform.y, transform.z, spawnType.skinChoice);
                    successfullyCreatedEntity = true;
                } 
                else if (spawnType.entityToSpawn == "test_cube") {
                    newEntity = entityFactory.createTestCube(transform.x, transform.y, transform.z, spawnType.skinChoice);
                    successfullyCreatedEntity = true;

                }// Add your other entity types here (e.g., "enemy_goblin", "npc_merchant")
                else if (spawnType.entityToSpawn == "npc")
                {
                    std::string dialogueSetId = "";

                    if (componentStorage.HasComponent<DialogueSource>(entity))
                    {
                        dialogueSetId = componentStorage.GetComponent<DialogueSource>(entity).dialogueSetId;
                    }

                    newEntity = entityFactory.createNPC(
                        transform.x,
                        transform.y,
                        transform.z,
                        dialogueSetId
                    );

                    successfullyCreatedEntity = true;
                }
                // Add your other entity types here (e.g., "enemy_goblin", "npc_merchant")
                else if (spawnType.entityToSpawn == "npc")
                {
                    std::string dialogueSetId = "";

                    if (componentStorage.HasComponent<DialogueSource>(entity))
                    {
                        dialogueSetId = componentStorage.GetComponent<DialogueSource>(entity).dialogueSetId;
                    }

                    newEntity = entityFactory.createNPC(
                        transform.x,
                        transform.y,
                        transform.z,
                        dialogueSetId
                    );
                }

                if (successfullyCreatedEntity && currentScene != nullptr) {

                    // If the JSON gave the spawner custom Health, pass it to the real enemy
                    if (componentStorage.HasComponent<Health>(entity)) {
                        if (componentStorage.HasComponent<Health>(newEntity)) {
                            componentStorage.GetComponent<Health>(newEntity) = componentStorage.GetComponent<Health>(entity);
                        } else {
                            componentStorage.AddComponent(newEntity, componentStorage.GetComponent<Health>(entity));
                        }
                        // Remove from spawner so the spawner doesn't take damage!
                        componentStorage.RemoveComponent<Health>(entity); 
                    }

                    // If the JSON gave the spawner custom Attack, pass it to the real enemy
                    if (componentStorage.HasComponent<Attack>(entity)) {
                        if (componentStorage.HasComponent<Attack>(newEntity)) {
                            componentStorage.GetComponent<Attack>(newEntity) = componentStorage.GetComponent<Attack>(entity);
                        } else {
                            componentStorage.AddComponent(newEntity, componentStorage.GetComponent<Attack>(entity));
                        }
                        // Remove from spawner so the spawner doesn't become a ghost attacker!
                        componentStorage.RemoveComponent<Attack>(entity); 
                    }

                    currentScene->addEntityToScene(newEntity);
                    spawnType.hasSpawned = true;
                }else if (!successfullyCreatedEntity){
                    // 6. Mark it as spawned so we don't spawn infinite copies!
                    spawnType.hasSpawned = true;

                }
            }
        }
    }
}