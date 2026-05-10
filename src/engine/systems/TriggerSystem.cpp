#include "TriggerSystem.h"

#include "../scene/Scene.h"
#include "../ecs/Components.h"


#include "raylib.h"

#include <iostream>
#include <cmath>

bool TriggerSystem::overlapsTrigger(
    float entityX,
    float entityY,
    float entityZ,
    float width,
    float height,
    float depth,
    float triggerX,
    float triggerY,
    float triggerZ
)
{
    BoundingBox entityBox = {
        {
            entityX - width / 2.0f,
            entityY - height / 2.0f,
            entityZ - depth / 2.0f
        },
        {
            entityX + width / 2.0f,
            entityY + height / 2.0f,
            entityZ + depth / 2.0f
        }
    };

    BoundingBox triggerBox = {
        { triggerX - 0.5f, triggerY - 0.5f, triggerZ - 0.5f },
        { triggerX + 0.5f, triggerY + 0.5f, triggerZ + 0.5f }
    };

    return CheckCollisionBoxes(entityBox, triggerBox);
}

void TriggerSystem::update(Scene& scene, EventBus& eventBus)
{
    ComponentStorage& components = scene.getComponentStorage();

    auto& players = components.GetComponents<PlayerInput>();

    for (auto& [entity, input] : players)
    {
        if (!components.HasComponent<TransformComponent>(entity))
            continue;

        if (!components.HasComponent<Collider>(entity))
            continue;

        auto& transform = components.GetComponent<TransformComponent>(entity);
        auto& collider = components.GetComponent<Collider>(entity);

        const SceneData& data = scene.getData();

        for (const auto& cube : data.cubes)
        {
            if (!cube.trigger)
                continue;

            float cubeX = static_cast<float>(cube.position.x);
            float cubeY = static_cast<float>(cube.position.y);
            float cubeZ = static_cast<float>(cube.position.z);

            if (std::fabs(cubeX - transform.x) > 2.0f) continue;
            if (std::fabs(cubeZ - transform.z) > 2.0f) continue;

            if (overlapsTrigger(
                transform.x,
                transform.y,
                transform.z,
                collider.width,
                collider.height,
                collider.depth,
                cubeX,
                cubeY,
                cubeZ
            ))
            {
                SceneTransitionEvent event;
                event.targetScene = cube.targetScene;
                eventBus.publish(SceneTransitionEvent{
                    cube.targetScene,
                    cube.targetSpawn
                });
            }
        }
    }
}