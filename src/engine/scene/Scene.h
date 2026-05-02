#pragma once
#include <string>
#include "raylib.h"
#include "SceneData.h"

#include "engine/ecs/EntityManager.h"
#include "engine/ecs/Components.h"
#include <vector>

class Scene
{
public:
    bool load(const std::string& scenePath);
    void update(float dt);
    void render(const Camera3D& camera);
    void unload();
private:
    std::string name;

    SceneData data;

    EntityManager entityManager;
    std::vector <TransformComponent> transforms;
    //std::vector <Renderer> renders;
    std::vector <Entity> activeEntitiesList;

    void renderEntities();
};