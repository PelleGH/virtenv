#include "RenderSystem.h"
#include "../scene/Scene.h"
#include "raylib.h"

#include <cstdint>

void RenderSystem::render(Scene& scene, ResourceManager& resourceManager, const Camera3D& camera)
{
    BeginMode3D(camera);

    renderGrid(scene, resourceManager);
    renderEntities(scene, resourceManager);

    EndMode3D();
}

void RenderSystem::renderEntities(Scene& scene, ResourceManager& resourceManager)
{
    const auto& activeEntities = scene.getActiveEntities();
    ComponentStorage& componentStorage = scene.getComponentStorage();
    EntityManager& entityManager = scene.getEntityManager();

    for (Entity e : activeEntities)
    {
        if (entityManager.isAlive(e) &&
            componentStorage.HasComponent<TransformComponent>(e) &&
            componentStorage.HasComponent<Renderer>(e))
        {
            const TransformComponent& tf = componentStorage.GetComponent<TransformComponent>(e);

            Vector3 pos = { tf.x, tf.y, tf.z };

            auto& renderer = componentStorage.GetComponent<Renderer>(e);

            if (renderer.modelID != "" && resourceManager.hasModel(renderer.modelID)) 
            {
                DrawModel(resourceManager.GetModel(renderer.modelID), pos, renderer.width, WHITE);
            }else {

                DrawCube(pos, renderer.width, renderer.height, renderer.depth, renderer.color);
                DrawCubeWires(pos, renderer.width, renderer.height, renderer.depth, MAROON);
            }
        }
    }
}
void RenderSystem::renderGrid(Scene& scene, ResourceManager& resourceManager)
{
    const SceneData& data = scene.getData();

    for (const auto& cube : data.cubes)
    {
        Vector3 pos = {
            (float)cube.position.x,
            (float)cube.position.y,
            (float)cube.position.z
        };
        Color cubeColor = GRAY;

        if (cube.trigger && !cube.targetScene.empty())
        {
            cubeColor = GREEN;
            DrawCube(pos, 1.0f, 1.0f, 1.0f, cubeColor);
            DrawCubeWires(pos, 1.0f, 1.0f, 1.0f, BLACK);
        }else{

            if (resourceManager.hasModel("wall_model")) {
                Model wall = resourceManager.GetModel("wall_model");

                DrawModel(wall, pos, 1.0f, WHITE);
                DrawCubeWires(pos, 1.0f, 1.0f, 1.0f, BLACK);

            }else {

                DrawCube(pos, 1.0f, 1.0f, 1.0f, MAGENTA);
                DrawCubeWires(pos, 1.0f, 1.0f, 1.0f, BLACK);
            }

        } 
    }
}