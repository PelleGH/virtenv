#include "Scene.h"
#include "SceneLoader.h"

#include <iostream>

bool Scene::load(const std::string& scenePath)
{
    if (!SceneLoader::loadFromFile(scenePath, data))
        return false;

    std::cout << "Loaded scene: " << data.name << '\n';
    std::cout << "Cubes: " << data.cubes.size() << '\n';

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

void Scene::render(const Camera3D& camera)
{
    BeginMode3D(camera);

    for (const auto& cube : data.cubes)
    {
        Vector3 pos = {
            (float)cube.position.x,
            (float)cube.position.y,
            (float)cube.position.z
        };

        DrawCube(pos, 1.0f, 1.0f, 1.0f, GRAY);
        DrawCubeWires(pos, 1.0f, 1.0f, 1.0f, BLACK);
    }

    EndMode3D();
}

void Scene::unload()
{
    std::cout << "Scene unloaded: " << data.name << '\n';
}