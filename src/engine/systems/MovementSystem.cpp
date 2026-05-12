#include "MovementSystem.h"
#include "../scene/Scene.h"
#include "../ecs/Components.h"

#include <cmath>

void MovementSystem::update(Scene& scene, float dt)
{
    ComponentStorage& components = scene.getComponentStorage();
    auto& velocities = components.GetComponents<Velocity>();

    for (auto& [entity, vel] : velocities)
    {
        if (!components.HasComponent<TransformComponent>(entity)) continue;

        TransformComponent& transform = components.GetComponent<TransformComponent>(entity);

        transform.previousX = transform.x;
        transform.previousY = transform.y;
        transform.previousZ = transform.z;

        transform.x += vel.x * vel.speed * dt;
        transform.z += vel.z * vel.speed * dt;
    }
}