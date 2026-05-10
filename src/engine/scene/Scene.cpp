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
    auto loadCubeModel = [&](const std::string& texID, const std::string& imgPath, const std::string& modelID, float size){
        resourceManager.LoadTexture2D(texID, imgPath);
        Model model = LoadModelFromMesh(GenMeshCube(size, size, size));
        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = resourceManager.GetTexture(texID);
        resourceManager.AddModel(modelID, model);
    };

    loadCubeModel("wall_tex",   "src/engine/assets/wall_texture.jpg", "wall_model",    1.0f);
    loadCubeModel("player_tex", "src/engine/assets/player1.jpg", "player_model1", 0.8f);
    loadCubeModel("player_tex2","src/engine/assets/player2.jpg", "player_model2", 0.8f);
    loadCubeModel("enemy_tex1", "src/engine/assets/enemy.jpg", "test_cube",     0.8f);
    loadCubeModel("enemy_tex2", "src/engine/assets/enemy2.jpg", "test_cube2",    0.8f);
    loadCubeModel("enemy_tex3", "src/engine/assets/enemy3.jpg", "test_cube3",    0.8f);

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
    componentStorage.RegisterComponent<ConditionalBlocker>();
    componentStorage.RegisterComponent<DialogueSource>();
    //componentStorage.RegisterComponent<Health>();
    //componentStorage.RegisterComponent<Attack>();
    componentStorage.RegisterComponent<Collider>();
    componentStorage.RegisterComponent<SceneTransition>();

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
    EntityFactory factory(entityManager, componentStorage);

    PlayerSpawn spawn;

    auto it = data.playerSpawns.find(spawnId);

    if (it != data.playerSpawns.end())
    {
        spawn = it->second;
    }
    else
    {
        auto defaultIt = data.playerSpawns.find("default");

        if (defaultIt != data.playerSpawns.end())
        {
            spawn = defaultIt->second;
        }
    }

    Entity player = factory.createPlayer(spawn.x, spawn.y, spawn.z);
    addEntityToScene(player);

    return player;
}
void Scene::update(float dt)
{
    std::cout << "Scene: " << data.name << '\n';

    for (const auto& cube : data.cubes)
    {
        Texture2D blockMaterial = resourceManager.GetTexture("wall_tex");

        std::cout << "Cube at ("
                  << cube.position.x << ", "
                  << cube.position.y << ", "
                  << cube.position.z << ") type: "
                  << cube.type << '\n';
    }
}

void Scene::unload()
{
    resourceManager.clear();
    
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

ResourceManager& Scene::getResourceManager() { 
    return resourceManager; 
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