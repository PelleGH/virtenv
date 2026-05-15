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

bool CollisionSystem::collidesWithGridDoor(
    Scene& scene,
    float x, float y, float z,
    float width, float height, float depth
)
{
    BoundingBox entityBox = {
        { x - width / 2.0f, y - height / 2.0f, z - depth / 2.0f },
        { x + width / 2.0f, y + height / 2.0f, z + depth / 2.0f }
    };

    const SceneData& data = scene.getData();

    for (const auto& cube : data.cubes)
    {
        // ONLY check cubes that are doors
        if (cube.type != "door")
            continue;

        float cubeX = static_cast<float>(cube.position.x);
        float cubeZ = static_cast<float>(cube.position.z);
        
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
        if (shouldIgnoreEntityCollision(components, self, otherEntity))
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
void CollisionSystem::update(Scene& scene, EventBus& eventBus)
{
    ComponentStorage& components = scene.getComponentStorage();
    auto& colliders = components.GetComponents<Collider>();

    for (auto& [entity, collider] : colliders)
    {
        if (!collider.enabled)
            continue;

        if (!components.HasComponent<Velocity>(entity))
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
                transform.previousX,
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
                transform.previousX,
                transform.previousY,
                transform.z,
                collider.width,
                collider.height,
                collider.depth
            );

        if (zOnlyBlocked)
            transform.z = transform.previousZ;
    }

    // Compare every collider against every other collider
    for (auto itA = colliders.begin(); itA != colliders.end(); ++itA)
    {
        Entity entityA = itA->first;
        auto& colA = itA->second;

        if (!components.HasComponent<TransformComponent>(entityA)) continue;
        auto& transA = components.GetComponent<TransformComponent>(entityA);

        //TRIGGER VS WALL or DOOR CHECK
        if (colA.isTrigger)
        {
            bool hitSolid = collidesWithSolidGrid(scene, transA.x, transA.y, transA.z, colA.width, colA.height, colA.depth);
            bool hitDoor = collidesWithGridDoor(scene, transA.x, transA.y, transA.z, colA.width, colA.height, colA.depth);

            // If it hits a solid wall OR a door, destroy it!
            if (hitSolid || hitDoor)
            {
                OverlapEvent overlap;
                overlap.entityA = entityA;
                overlap.hitWall = true; 
                eventBus.publish(overlap);
            }
        }

        // Start itB at the next item so we don't compare A vs B and B vs A!
        auto itB = itA;
        ++itB;
        //TRIGGER VS ENTITY CHECK
        for (; itB != colliders.end(); ++itB)
        {
            Entity entityB = itB->first;
            auto& colB = itB->second;

            if (!components.HasComponent<TransformComponent>(entityB)) continue;
            auto& transB = components.GetComponent<TransformComponent>(entityB);

            // If AT LEAST ONE of them is a trigger, check for overlap
            if (colA.isTrigger || colB.isTrigger)
            {
                if (overlapsBox(
                    transA.x, transA.y, transA.z, colA.width, colA.height, colA.depth,
                    transB.x, transB.y, transB.z, colB.width, colB.height, colB.depth))
                {
                    // They touched! Tell the EventBus!
                    OverlapEvent overlap;
                    overlap.entityA = entityA;
                    overlap.entityB = entityB;
                    eventBus.publish(overlap);
                }
            }
        }
    }
}
bool CollisionSystem::isMoveBlocked(
    Scene& scene,
    Entity entity,
    float directionX,
    float directionZ,
    float distance
) {
    auto& components = scene.getComponentStorage();

    if (!components.HasComponent<TransformComponent>(entity)) return false;
    if (!components.HasComponent<Collider>(entity)) return false;

    auto& transform = components.GetComponent<TransformComponent>(entity);
    auto& collider = components.GetComponent<Collider>(entity);

    float testX = transform.x + directionX * distance;
    float testZ = transform.z + directionZ * distance;

    return collidesWithSolidGrid(
        scene,
        testX,
        transform.y,
        testZ,
        collider.width,
        collider.height,
        collider.depth
    )
    ||
    collidesWithSolidEntities(
        scene,
        entity,
        testX,
        transform.y,
        testZ,
        collider.width,
        collider.height,
        collider.depth
    );
}
bool CollisionSystem::shouldIgnoreEntityCollision(
    ComponentStorage& components,
    Entity self,
    Entity other
)
{
    bool selfIsPlayer = components.HasComponent<PlayerInput>(self);
    bool otherIsPlayer = components.HasComponent<PlayerInput>(other);

    bool selfIsEnemy = components.HasComponent<AIController>(self);
    bool otherIsEnemy = components.HasComponent<AIController>(other);

    // Let player and enemies overlap physically.
    // Combat still uses Attack range, so enemies can still damage the player.
    if ((selfIsPlayer && otherIsEnemy) ||
        (selfIsEnemy && otherIsPlayer))
    {
        return true;
    }

    return false;
}