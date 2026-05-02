#include "Scene.h"
#include "SceneLoader.h"

#include <iostream>
#include <cstdint>

bool Scene::load(const std::string& scenePath)
{
    if (!SceneLoader::loadFromFile(scenePath, data))
        return false;

    std::cout << "Loaded scene: " << data.name << '\n';
    std::cout << "Cubes: " << data.cubes.size() << '\n';
    
    componentStorage.RegisterComponent<TransformComponent>();
    componentStorage.RegisterComponent<Renderer>();
    componentStorage.RegisterComponent<PlayerInput>();

    // Create player (hardocoded for now, will be data-driven later)
    Entity player = entityManager.createEntity();
    activeEntitiesList.push_back(player);
    Renderer r;
    r.color = BLUE;
    componentStorage.AddComponent(player, TransformComponent{0.0f, 1.0f, 0.0f});
    componentStorage.AddComponent(player, r);
    componentStorage.AddComponent(player, PlayerInput{});
    //Hardcoded at the moment to verify that entities is generated and visible
    for (int i=0; i <= 3; i++){
        
        //Creates entities
        Entity newEntity = entityManager.createEntity(); 
        
        //Add the entity to the list
        activeEntitiesList.push_back(newEntity);

        //Checks that transforms "list" have enough space to handle all entities
        componentStorage.AddComponent(newEntity, TransformComponent{
            i * 1.0f,
            2.0f,
            0.0f
        });

        componentStorage.AddComponent(newEntity, Renderer{});
    }
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