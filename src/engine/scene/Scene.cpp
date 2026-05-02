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

    //Hardcoded at the moment to verify that entities is generated and visible
    for (int i=0; i <= 3; i++){
        
        //Creates entities
        Entity newEntity = entityManager.createEntity(); 
        
        //Add the entity to the list
        activeEntitiesList.push_back(newEntity);

        //Checks that transforms "list" have enough space to handle all entities
        if (transforms.size() <= newEntity.getIndex()) {
            transforms.resize(newEntity.getIndex() + 1);
        }

        //Gets entities and places them in the world
        TransformComponent& tf = transforms[newEntity.getIndex()];
        tf.x = i* 1.0f;
        tf.y = 2.0f;
        tf.z = 0.0f;
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

    renderEntities();
    EndMode3D();
}

void Scene::unload()
{
    std::cout << "Scene unloaded: " << data.name << '\n';
}

void Scene::renderEntities(){

    for (Entity e : activeEntitiesList){
        if (entityManager.isAlive(e)){

            //Gets entity ID and their coordinates
            std::uint32_t index = e.getIndex();
            TransformComponent& tf = transforms[index];

            //Renderer& r = render[index];

            //Translate coordinates for Raylib
            Vector3 pos = {
                tf.x,
                tf.y,
                tf.z
            };

            DrawCube(pos, 0.5f, 0.5f, 0.5f, RED);
            DrawCubeWires(pos, 0.5f, 0.5f, 0.5f, MAROON);
        }
    }
}