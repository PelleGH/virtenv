#include "CollisionSystem.h"

#include "../scene/Scene.h"
#include "../ecs/Components.h"

#include "raylib.h"

bool CollisionSystem::collidesWithSolidGrid(
    Scene& scene,
    float x,
    float y,
    float z,
    float width,
    float height,
    float depth
)
{
    BoundingBox entityBox = {
        { x - width / 2.0f, y - height / 2.0f, z - depth / 2.0f },
        { x + width / 2.0f, y + height / 2.0f, z + depth / 2.0f }
    };

    const SceneData& data = scene.getData();

    for (const auto& cube : data.cubes)
    {
        if (!cube.solid)
            continue;

        float cubeX = static_cast<float>(cube.position.x);
        float cubeZ = static_cast<float>(cube.position.z);
        // check nearby cubes
        if (fabs(cubeX - x) > 2.0f) continue;
        if (fabs(cubeZ - z) > 2.0f) continue;

        BoundingBox cubeBox = {
            { cubeX - 0.5f, cube.position.y - 0.5f, cubeZ - 0.5f },
            { cubeX + 0.5f, cube.position.y + 0.5f, cubeZ + 0.5f }
        };

        if (CheckCollisionBoxes(entityBox, cubeBox))
            return true;
    }

    return false;
}
bool CollisionSystem::overlapsBox(
    float ax, float ay, float az,
    float aw, float ah, float ad,
    float bx, float by, float bz,
    float bw, float bh, float bd
)
{
    BoundingBox a = {
        { ax - aw / 2.0f, ay - ah / 2.0f, az - ad / 2.0f },
        { ax + aw / 2.0f, ay + ah / 2.0f, az + ad / 2.0f }
    };

    BoundingBox b = {
        { bx - bw / 2.0f, by - bh / 2.0f, bz - bd / 2.0f },
        { bx + bw / 2.0f, by + bh / 2.0f, bz + bd / 2.0f }
    };

    return CheckCollisionBoxes(a, b);
}

bool CollisionSystem::collidesWithSolidEntities(
    Scene& scene,
    Entity self,
    float x,
    float y,
    float z,
    float width,
    float height,
    float depth
)
{
    ComponentStorage& components = scene.getComponentStorage();
    auto& colliders = components.GetComponents<Collider>();

    for (auto& [otherEntity, otherCollider] : colliders)
    {
        if (!otherCollider.enabled)
            continue;

        if (otherCollider.isTrigger)
            continue;
        if (otherEntity == self)
            continue;

        if (otherCollider.isTrigger)
            continue;

        if (!components.HasComponent<TransformComponent>(otherEntity))
            continue;

        TransformComponent& otherTransform =
            components.GetComponent<TransformComponent>(otherEntity);

        if (overlapsBox(
            x, y, z,
            width, height, depth,
            otherTransform.x, otherTransform.y, otherTransform.z,
            otherCollider.width, otherCollider.height, otherCollider.depth
        ))
        {
            return true;
        }
    }

    return false;
}
void CollisionSystem::update(Scene& scene)
{
    ComponentStorage& components = scene.getComponentStorage();
    auto& colliders = components.GetComponents<Collider>();

    for (auto& [entity, collider] : colliders)
    {
        if (!collider.enabled)
            continue;

        if (!components.HasComponent<PlayerInput>(entity))
            continue;
        if (collider.isTrigger)
            continue;

        if (!components.HasComponent<TransformComponent>(entity))
            continue;

        TransformComponent& transform = components.GetComponent<TransformComponent>(entity);

        bool currentPositionBlocked =
            collidesWithSolidGrid(
                scene,
                transform.x,
                transform.y,
                transform.z,
                collider.width,
                collider.height,
                collider.depth
            )
            ||
            collidesWithSolidEntities(
                scene,
                entity,
                transform.x,
                transform.y,
                transform.z,
                collider.width,
                collider.height,
                collider.depth
            );

        if (!currentPositionBlocked)
            continue;

        bool xOnlyBlocked =
            collidesWithSolidGrid(
                scene,
                transform.x,
                transform.previousY,
                transform.previousZ,
                collider.width,
                collider.height,
                collider.depth
            )
            ||
            collidesWithSolidEntities(
                scene,
                entity,
                transform.x,
                transform.previousY,
                transform.previousZ,
                collider.width,
                collider.height,
                collider.depth
            );

        if (xOnlyBlocked)
            transform.x = transform.previousX;

        bool zOnlyBlocked =
            collidesWithSolidGrid(
                scene,
                transform.x,
                transform.previousY,
                transform.z,
                collider.width,
                collider.height,
                collider.depth
            )
            ||
            collidesWithSolidEntities(
                scene,
                entity,
                transform.x,
                transform.previousY,
                transform.z,
                collider.width,
                collider.height,
                collider.depth
            );

        if (zOnlyBlocked)
            transform.z = transform.previousZ;
    }
}