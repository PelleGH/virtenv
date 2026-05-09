#include "Scene.h"
#include "SceneLoader.h"
#include "../ecs/EntityFactory.h"
#include "../systems/SpawnSystem.h"

#include <iostream>
#include <fstream>
#include <cstdint>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool Scene::load(const std::string& scenePath)
{
    // 1. Ladda in den statiska kartan (väggar, golv, dörrar)
    if (!SceneLoader::loadFromFile(scenePath, data))
        return false;

    std::cout << "Loaded scene: " << data.name << '\n';
    std::cout << "Cubes: " << data.cubes.size() << '\n';
    
    // 2. Registrera alla komponenter du använder i scenen
    componentStorage.RegisterComponent<TransformComponent>();
    componentStorage.RegisterComponent<Renderer>();
    componentStorage.RegisterComponent<PlayerInput>();
    componentStorage.RegisterComponent<SpawnType>();

    componentStorage.RegisterComponent<Health>();
    componentStorage.RegisterComponent<Attack>();
    componentStorage.RegisterComponent<Collider>();
    componentStorage.RegisterComponent<SceneTransition>();

    EntityFactory factory(entityManager, componentStorage);

    // 4. Ladda in dina spawners från JSON-filen
    std::ifstream file("src/engine/ecs/saved_entities.json");
    if (file.is_open()) {
        json entitiesJson;
        file >> entitiesJson;
        for (const auto& entityData : entitiesJson) {
            // Skapar de "osynliga" spawner-entiteterna
            Entity loadedEntity = factory.deserialize(entityData);
            addEntityToScene(loadedEntity);
        }
    } else {
        std::cout << "Warning: Could not find saved_entities.json\n";
    }

    // 5. Kör SpawnSystemet!
    // Det här systemet letar nu upp alla nyskapade SpawnTypes, läser deras X/Y/Z, 
    // och ber EntityFactory att bygga den riktiga spelaren och de riktiga testkuberna.
    SpawnSystem spawnSystem(componentStorage, factory);
    spawnSystem.Update(this);

    return true;
}

void Scene::update(float dt)
{
    std::cout << "Scene: " << data.name << '\n';

    for (const auto& cube : data.cubes)
    {
        std::cout << "Cube at ("
                  << cube.position.x << ", "
                  << cube.position.y << ", "
                  << cube.position.z << ") type: "
                  << cube.type << '\n';
    }
}

void Scene::unload()
{
    data.cubes.clear();
    activeEntitiesList.clear();

    entityManager = EntityManager();
    componentStorage = ComponentStorage();
}

const SceneData& Scene::getData() const
{
    return data;
}

const std::vector<Entity>& Scene::getActiveEntities() const
{
    return activeEntitiesList;
}

ComponentStorage& Scene::getComponentStorage()
{
    return componentStorage;
}

EntityManager& Scene::getEntityManager()
{
    return entityManager;
}

void Scene::saveState()
{
    // The Scene already knows about its own managers, so it's super clean!
    EntityFactory factory(entityManager, componentStorage);
    factory.saveEntitiesToFile(activeEntitiesList);
}

void Scene::addEntityToScene(Entity entity) 
{
    activeEntitiesList.push_back(entity);
}

void Scene::queueEntityDestruction(Entity entity)
{
    std::cout << "Scene queuing entity for destruction: " << entity.getIndex() << '\n';
    // Use your exact existing queue method!
    entityManager.destroyEntityQueue(entity); 
}

void Scene::cleanupDestroyedEntities()
{
    // 1. Tell your EntityManager to process the queue and update generations
    entityManager.destroyEntity();

    // 2. Safely remove any dead entities from your active list
    activeEntitiesList.erase(
        std::remove_if(activeEntitiesList.begin(), activeEntitiesList.end(),
            [this](Entity e) { 
                return !entityManager.isAlive(e); 
            }),
        activeEntitiesList.end()
    );

    // Note: If your ComponentStorage has a method to clear an entity's components, 
    // you would call it inside the lambda above!
}