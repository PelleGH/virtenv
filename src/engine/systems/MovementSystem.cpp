#include "MovementSystem.h"
#include "../scene/Scene.h"
#include "../ecs/Components.h"

#include <cmath>

void MovementSystem::update(Scene& scene, float dt)
{
    ComponentStorage& components = scene.getComponentStorage();
    auto& inputs = components.GetComponents<PlayerInput>();

    for (auto& [entity, input] : inputs)
    {
        if (!components.HasComponent<TransformComponent>(entity))
            continue;

        TransformComponent& transform =
            components.GetComponent<TransformComponent>(entity);

        float moveX = 0.0f;
        float moveZ = 0.0f;

        if (input.right) moveX += 1.0f;
        if (input.left)  moveX -= 1.0f;
        if (input.up)    moveZ -= 1.0f;
        if (input.down)  moveZ += 1.0f;

        float length = std::sqrt(moveX * moveX + moveZ * moveZ);

        if (length > 0.0f)
        {
            moveX /= length;
            moveZ /= length;
        }

        float speed = 3.0f;

        transform.x += moveX * speed * dt;
        transform.z += moveZ * speed * dt;
    }
}