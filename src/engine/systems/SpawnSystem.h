#pragma once
#include "engine/ecs/ComponentStorage.h"
#include "engine/ecs/EntityFactory.h"

class Scene;

class SpawnSystem {
private:
    ComponentStorage& componentStorage;
    EntityFactory& entityFactory;

public:
    SpawnSystem(ComponentStorage& cs, EntityFactory& ef);
    
    void Update(Scene* currentScene);
};