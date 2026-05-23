#include "RenderSystem.h"
#include "../scene/Scene.h"
#include "raylib.h"

#include <cstdint>

void RenderSystem::render(Scene& scene, ResourceManager& resourceManager, const Camera3D& camera)
{
    RenderOptions options;
    render(scene, resourceManager, camera, options);
}

void RenderSystem::render(Scene& scene, ResourceManager& resourceManager, const Camera3D& camera, const RenderOptions& options)
{
    BeginMode3D(camera);

    renderGrid(scene, resourceManager);
    renderEntities(scene, resourceManager);

    EndMode3D();

    if (options.drawHealthBars)
        renderHealthBars(scene, camera);

    if (options.drawPlayerUI)
        renderPlayerHealthBar(scene);
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
                // Draw the 3Dmodel or the cube with skin
                DrawModel(resourceManager.GetModel(renderer.modelID), pos, renderer.scale, WHITE);
            }else {

                // If no modelID is written in JSON, draw a colored cube
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
        if (!cube.visible)
        {
            continue;
        }
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

            if (cube.modelID != "" && resourceManager.hasModel(cube.modelID)) {
                Model wall = resourceManager.GetModel(cube.modelID);

                DrawModel(wall, pos, 1.0f, WHITE);
                DrawCubeWires(pos, 1.0f, 1.0f, 1.0f, BLACK);

            }else {

                DrawCube(pos, 1.0f, 1.0f, 1.0f, MAGENTA);
                DrawCubeWires(pos, 1.0f, 1.0f, 1.0f, BLACK);
            }

        } 
    }
}
void RenderSystem::renderHealthBars(Scene& scene, const Camera3D& camera)
{
    const auto& activeEntities = scene.getActiveEntities();
    ComponentStorage& components = scene.getComponentStorage();
    EntityManager& entityManager = scene.getEntityManager();

    for (Entity e : activeEntities)
    {
        if (!entityManager.isAlive(e)) continue;

        // Only enemies for now:
        if (!components.HasComponent<AIController>(e)) continue;

        if (!components.HasComponent<TransformComponent>(e)) continue;
        if (!components.HasComponent<Health>(e)) continue;

        const auto& tf = components.GetComponent<TransformComponent>(e);
        const auto& health = components.GetComponent<Health>(e);

        if (health.current <= 0) continue;

        float hpPercent = (float)health.current / (float)health.max;
        if (hpPercent > 1.0f) hpPercent = 1.0f;
        if (hpPercent < 0.0f) hpPercent = 0.0f;

        Vector3 worldPos = {
            tf.x,
            tf.y + tf.height + 0.4f,
            tf.z
        };

        Vector2 screenPos = GetWorldToScreen(worldPos, camera);

        float width = 40.0f;
        float height = 6.0f;

        float x = screenPos.x - width / 2.0f;
        float y = screenPos.y;

        DrawRectangle((int)x, (int)y, (int)width, (int)height, DARKGRAY);
        DrawRectangle((int)x, (int)y, (int)(width * hpPercent), (int)height, RED);
        DrawRectangleLines((int)x, (int)y, (int)width, (int)height, BLACK);
    }
}
void RenderSystem::renderPlayerHealthBar(Scene& scene)
{
    ComponentStorage& components = scene.getComponentStorage();

    for (Entity e : scene.getActiveEntities())
    {
        if (!components.HasComponent<PlayerInput>(e)) continue;
        if (!components.HasComponent<Health>(e)) continue;

        const auto& health = components.GetComponent<Health>(e);

        float hpPercent = (float)health.current / (float)health.max;
        if (hpPercent < 0.0f) hpPercent = 0.0f;
        if (hpPercent > 1.0f) hpPercent = 1.0f;

        int x = 20;
        int y = 20;
        int width = 220;
        int height = 22;

        DrawRectangle(x, y, width, height, DARKGRAY);
        DrawRectangle(x, y, (int)(width * hpPercent), height, RED);
        DrawRectangleLines(x, y, width, height, BLACK);

        DrawText(
            TextFormat("HP: %d / %d", health.current, health.max),
            x + 8,
            y + 4,
            14,
            WHITE
        );

        break;
    }
}