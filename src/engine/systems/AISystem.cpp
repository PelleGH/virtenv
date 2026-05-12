#include "AISystem.h"
#include "../scene/Scene.h"
#include "../ecs/Components.h"
#include "../messaging/EventBus.h"
#include "../ecs/EntityFactory.h"
#include "raylib.h"

void AISystem::update(Scene& scene, EventBus& eventBus)
{
    ComponentStorage& components = scene.getComponentStorage();
    float dt = GetFrameTime();

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