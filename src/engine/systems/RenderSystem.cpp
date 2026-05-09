#include "RenderSystem.h"
#include "../scene/Scene.h"
#include "raylib.h"

#include <cstdint>

void RenderSystem::render(Scene& scene, const Camera3D& camera)
{
    BeginMode3D(camera);

    renderGrid(scene);
    renderEntities(scene);

    EndMode3D();
}

void RenderSystem::renderEntities(Scene& scene)
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

            DrawCube(pos, renderer.width, renderer.height, renderer.depth, renderer.color);
            DrawCubeWires(pos, renderer.width, renderer.height, renderer.depth, MAROON);
        }
    }
}
void RenderSystem::renderGrid(Scene& scene)
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
        }
        DrawCube(pos, 1.0f, 1.0f, 1.0f, cubeColor);
        DrawCubeWires(pos, 1.0f, 1.0f, 1.0f, BLACK);
    }
}