#pragma once
#include <string>
#include "raylib.h"
#include "SceneData.h"

#include "engine/ecs/EntityManager.h"
#include "engine/ecs/Components.h"
#include "engine/ecs/ComponentStorage.h"
#include <vector>

class Scene
{
public:
    bool load(const std::string& scenePath);
    void update(float dt);
    void unload();
    void saveState();
    void addEntityToScene(Entity entity);

    const SceneData& getData() const;
    const std::vector<Entity>& getActiveEntities() const;

    EntityManager& getEntityManager();
    ComponentStorage& getComponentStorage();
private:
    std::string name;

    SceneData data;

    EntityManager entityManager;
    ComponentStorage componentStorage;
    //std::vector <Renderer> renders;
    std::vector <Entity> activeEntitiesList;

};