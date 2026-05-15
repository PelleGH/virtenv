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

    //std::cout << "Loaded scene: " << data.name << '\n';
    //std::cout << "Cubes: " << data.cubes.size() << '\n';
    
    // 2. Registrera alla komponenter du använder i scenen
    registerComponents();

    EntityFactory factory(entityManager, componentStorage);

    // 4. Ladda in dina spawners från JSON-filen
    
    for (const auto& entityData : data.entities)
    {
        Entity loadedEntity = factory.deserialize(entityData);
        addEntityToScene(loadedEntity);
    }

    SpawnSystem spawnSystem(componentStorage, factory);
    spawnSystem.Update(this);

    return true;
}

Entity Scene::spawnPlayerAt(const std::string& spawnId)
{
    PlayerSpawn spawn;

    auto it = data.playerSpawns.find(spawnId);
    if (it != data.playerSpawns.end())
        spawn = it->second;
    else if (data.playerSpawns.find("default") != data.playerSpawns.end())
        spawn = data.playerSpawns["default"];

    for (Entity e : activeEntitiesList)
    {
        if (entityManager.isAlive(e) &&
            componentStorage.HasComponent<PlayerInput>(e) &&
            componentStorage.HasComponent<TransformComponent>(e))
        {
            auto& transform = componentStorage.GetComponent<TransformComponent>(e);
            transform.x = spawn.x;
            transform.y = spawn.y;
            transform.z = spawn.z;

            return e;
        }
    }

    EntityFactory factory(entityManager, componentStorage);
    Entity player = factory.createPlayer(spawn.x, spawn.y, spawn.z, spawn.skinChoice);
    addEntityToScene(player);

    return player;
}

void Scene::update(float dt)
{
    //std::cout << "Scene: " << data.name << '\n';

    for (const auto& cube : data.cubes)
    {
        
        //std::cout << "Cube at ("
        //          << cube.position.x << ", "
        //          << cube.position.y << ", "
        //          << cube.position.z << ") type: "
        //          << cube.type << '\n';
    }
}

void Scene::unload()
{
    data.cubes.clear();

    std::vector<Entity> keptEntities;

    for (Entity e : activeEntitiesList)
    {
        bool isPlayer =
            entityManager.isAlive(e) &&
            componentStorage.HasComponent<PlayerInput>(e);

        if (isPlayer)
        {
            keptEntities.push_back(e);
        }
        else
        {
            entityManager.destroyEntityQueue(e);
        }
    }

    entityManager.destroyEntity();

    for (Entity e : activeEntitiesList)
    {
        bool isPlayer =
            entityManager.isAlive(e) &&
            componentStorage.HasComponent<PlayerInput>(e);

        if (!isPlayer)
        {
            componentStorage.EntityDestroyed(e);
        }
    }

    activeEntitiesList = keptEntities;
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

    // 2. Safely remove any dead entities from your active list AND wipe their components
    activeEntitiesList.erase(
        std::remove_if(activeEntitiesList.begin(), activeEntitiesList.end(),
            [this](Entity e) { 
                if (!entityManager.isAlive(e)) {
                    // Tell the ComponentStorage to delete all data attached to this dead entity ID
                    componentStorage.EntityDestroyed(e); 
                    return true;
                }
                return false;
            }),
        activeEntitiesList.end()
    );
}
void Scene::registerComponents()
{
    if (componentsRegistered)
        return;

    componentStorage.RegisterComponent<TransformComponent>();
    componentStorage.RegisterComponent<Renderer>();
    componentStorage.RegisterComponent<PlayerInput>();
    componentStorage.RegisterComponent<SpawnType>();
    componentStorage.RegisterComponent<ConditionalBlocker>();
    componentStorage.RegisterComponent<DialogueSource>();
    componentStorage.RegisterComponent<Health>();
    componentStorage.RegisterComponent<Attack>();
    componentStorage.RegisterComponent<AIController>();
    componentStorage.RegisterComponent<Collider>();
    componentStorage.RegisterComponent<SceneTransition>();
    componentStorage.RegisterComponent<Velocity>();
    componentStorage.RegisterComponent<Projectile>();
    componentStorage.RegisterComponent<Interactable>();

    componentsRegistered = true;
}
Entity Scene::getPlayer()
{
    for (Entity e : activeEntitiesList)
    {
        if (entityManager.isAlive(e) &&
            componentStorage.HasComponent<PlayerInput>(e))
        {
            return e;
        }
    }

    return Entity{};
}
void Scene::resetPlayerAt(const std::string& spawnId)
{
    Entity player = getPlayer();

    if (!entityManager.isAlive(player))
        return;

    PlayerSpawn spawn;

    auto it = data.playerSpawns.find(spawnId);
    if (it != data.playerSpawns.end())
        spawn = it->second;
    else if (data.playerSpawns.find("default") != data.playerSpawns.end())
        spawn = data.playerSpawns["default"];

    if (componentStorage.HasComponent<TransformComponent>(player))
    {
        auto& t = componentStorage.GetComponent<TransformComponent>(player);
        t.x = spawn.x;
        t.y = spawn.y;
        t.z = spawn.z;

        t.previousX = spawn.x;
        t.previousY = spawn.y;
        t.previousZ = spawn.z;
    }

    if (componentStorage.HasComponent<Health>(player))
    {
        auto& h = componentStorage.GetComponent<Health>(player);
        h.current = h.max;
    }

    if (componentStorage.HasComponent<Velocity>(player))
    {
        auto& v = componentStorage.GetComponent<Velocity>(player);
        v.x = 0.0f;
        v.z = 0.0f;
    }

    // Later when inventory exists:
    // if (componentStorage.HasComponent<Inventory>(player))
    //     componentStorage.GetComponent<Inventory>(player).items.clear();
}