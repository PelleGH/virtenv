#pragma once
#include <string>
#include "raylib.h"
#include "SceneData.h"

#include "engine/ecs/EntityManager.h"
#include "engine/ecs/Components.h"
#include "engine/ecs/ComponentStorage.h"
#include "engine/resource/ResourceManager.h"
#include <vector>

class Scene
{
public:
    bool load(const std::string& scenePath);
    void update(float dt);
    void unload();
    void saveState();
    void addEntityToScene(Entity entity);
    Entity spawnPlayerAt(const std::string& spawnId);
    void queueEntityDestruction(Entity entity);
    void cleanupDestroyedEntities();
    void spawnItemDrop(float x, float y, float z, const std::string& itemId);

    const SceneData& getData() const;
    const std::vector<Entity>& getActiveEntities() const;

    EntityManager& getEntityManager();
    ComponentStorage& getComponentStorage();
    
    Entity getPlayer();
    void resetPlayerAt(const std::string& spawnId);
private:
    std::string name;

    SceneData data;

    EntityManager entityManager;
    ComponentStorage componentStorage;
    //std::vector <Renderer> renders;
    std::vector <Entity> activeEntitiesList;

    bool componentsRegistered = false;

    void registerComponents();


};