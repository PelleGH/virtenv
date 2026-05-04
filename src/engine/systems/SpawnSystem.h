#pragma once
#include "engine/ecs/ComponentStorage.h"
#include "engine/ecs/EntityFactory.h"

class SpawnSystem {
private:
    ComponentStorage& componentStorage;
    EntityFactory& entityFactory;

public:
    SpawnSystem(ComponentStorage& cs, EntityFactory& ef);
    
    void Update(); 
};