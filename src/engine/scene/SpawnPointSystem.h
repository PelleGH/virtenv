#pragma once
#include "engine/scene/Scene.h"
#include "engine/ecs/EntityFactory.h"

class SpawnPointSystem {
public:
    // Called once right after a scene loads
    void processSpawnPoints(Scene& scene, EntityFactory& factory);

    // ADD THIS LINE HERE:
    void saveSpawnData(Scene& scene, EntityFactory& factory, const std::string& filename);
};