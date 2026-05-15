#include "AISystem.h"
#include "../scene/Scene.h"
#include "../ecs/Components.h"
#include "../messaging/EventBus.h"
#include "../ecs/EntityFactory.h"
#include "raylib.h"

void AISystem::update(Scene& scene, EventBus& eventBus, float dt)
{
    
    ComponentStorage& components = scene.getComponentStorage();

    // 1. Find the Player entity to know where to attack
    Entity playerEntity;
    bool playerFound = false;
    for (auto& [entity, input] : components.GetComponents<PlayerInput>()) {
        playerEntity = entity;
        playerFound = true;
        break; // Assuming only one player
    }

    if (!playerFound || !components.HasComponent<TransformComponent>(playerEntity)) return;
    auto& playerTransform = components.GetComponent<TransformComponent>(playerEntity);

    // 2. Loop through all entities that have an Attack component
    for (auto& [entity, attackStats] : components.GetComponents<Attack>())
    {
        // Skip the player (the InputSystem handles the player's attacks)
        if (components.HasComponent<PlayerInput>(entity)) continue;
        if (!components.HasComponent<TransformComponent>(entity)) continue;

        auto& enemyTransform = components.GetComponent<TransformComponent>(entity);

        // Movement Logic
        if (components.HasComponent<AIController>(entity) &&
            components.HasComponent<Velocity>(entity))
        {
            auto& ai = components.GetComponent<AIController>(entity);
            auto& vel = components.GetComponent<Velocity>(entity);

            float dx = playerTransform.x - enemyTransform.x;
            float dz = playerTransform.z - enemyTransform.z;
            float distSq2D = dx * dx + dz * dz;
            float dist = std::sqrt(distSq2D);

            vel.x = 0.0f;
            vel.z = 0.0f;

            if (dist <= ai.aggroRange && dist > 0.001f)
            {
                float dirX = dx / dist;
                float dirZ = dz / dist;

                // --- Separation from nearby enemies ---
                float separationX = 0.0f;
                float separationZ = 0.0f;

                const float separationRadius = 1.3f;
                const float separationWeight = 1.2f;

                for (auto& [otherEntity, otherAttack] : components.GetComponents<Attack>())
                {
                    if (otherEntity == entity) continue;
                    if (components.HasComponent<PlayerInput>(otherEntity)) continue;
                    if (!components.HasComponent<TransformComponent>(otherEntity)) continue;

                    auto& otherTransform =
                        components.GetComponent<TransformComponent>(otherEntity);

                    float awayX = enemyTransform.x - otherTransform.x;
                    float awayZ = enemyTransform.z - otherTransform.z;

                    float awayDistSq = awayX * awayX + awayZ * awayZ;

                    if (awayDistSq > 0.001f &&
                        awayDistSq < separationRadius * separationRadius)
                    {
                        float awayDist = std::sqrt(awayDistSq);

                        separationX += awayX / awayDist;
                        separationZ += awayZ / awayDist;
                    }
                }

                if (!attackStats.isRanged)
                {
                    if (dist > attackStats.range)
                    {
                        float finalX = dirX + separationX * separationWeight;
                        float finalZ = dirZ + separationZ * separationWeight;

                        float finalLength = std::sqrt(finalX * finalX + finalZ * finalZ);

                        if (finalLength > 0.001f)
                        {
                            vel.x = finalX / finalLength;
                            vel.z = finalZ / finalLength;
                        }
                    }
                }
                else
                {
                    float moveX = 0.0f;
                    float moveZ = 0.0f;

                    if (dist > ai.preferredRange)
                    {
                        moveX = dirX;
                        moveZ = dirZ;
                    }
                    else if (dist < ai.minimumRange)
                    {
                        moveX = -dirX;
                        moveZ = -dirZ;
                    }

                    float finalX = moveX + separationX * separationWeight;
                    float finalZ = moveZ + separationZ * separationWeight;

                    float finalLength = std::sqrt(finalX * finalX + finalZ * finalZ);

                    if (finalLength > 0.001f)
                    {
                        vel.x = finalX / finalLength;
                        vel.z = finalZ / finalLength;
                    }
                }
            }
        }
        // Always tick the enemy's cooldown timer
        attackStats.timeSinceLastAttack += dt;

        // 3. Check distance to player
        float dx = playerTransform.x - enemyTransform.x;
        float dy = playerTransform.y - enemyTransform.y;
        float dz = playerTransform.z - enemyTransform.z;
        float distSquared = (dx*dx) + (dy*dy) + (dz*dz);
        float rangeSquared = attackStats.range * attackStats.range;

        // 4. If in range AND cooldown is finished, attack!
        if (distSquared <= rangeSquared && attackStats.timeSinceLastAttack >= attackStats.cooldown)
        {
            attackStats.timeSinceLastAttack = 0.0f; // Reset timer

            if (attackStats.isRanged) 
            {
                // Calculate normalized direction vector to player
                float distance = std::sqrt(distSquared);
                float dirX = dx / distance;
                float dirZ = dz / distance;

                float leftX = -dirZ;
                float leftZ = dirX;

                float rightX = dirZ;
                float rightZ = -dirX;
                // Spawn the physical projectile
                EntityFactory factory(scene.getEntityManager(), components);
                Entity proj = factory.createProjectile(
                    enemyTransform.x, 
                    enemyTransform.y, 
                    enemyTransform.z, 
                    dirX,                            // Just the X direction
                    dirZ,                            // Just the Z direction
                    attackStats.projectileSpeed,     
                    attackStats.damage, 
                    entity
                );
                scene.addEntityToScene(proj);
            } 
            else 
            {
                // Standard Melee Attack
                AttackEvent attackEvent;
                attackEvent.attacker = entity;
                attackEvent.attackRange = attackStats.range;
                attackEvent.damage = attackStats.damage;
                eventBus.publish(attackEvent);
            }
        }
    }
}
