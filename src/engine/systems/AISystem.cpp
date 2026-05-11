#include "AISystem.h"
#include "../scene/Scene.h"
#include "../ecs/Components.h"
#include "../messaging/EventBus.h"
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
        float dx = enemyTransform.x - playerTransform.x;
        float dy = enemyTransform.y - playerTransform.y;
        float dz = enemyTransform.z - playerTransform.z;
        float distSquared = (dx*dx) + (dy*dy) + (dz*dz);
        float rangeSquared = attackStats.range * attackStats.range;

        // 4. If in range AND cooldown is finished, attack!
        if (distSquared <= rangeSquared && attackStats.timeSinceLastAttack >= attackStats.cooldown)
        {
            attackStats.timeSinceLastAttack = 0.0f; // Reset timer

            AttackEvent attackEvent;
            attackEvent.attacker = entity;
            attackEvent.attackRange = attackStats.range;
            attackEvent.damage = attackStats.damage;
            
            eventBus.publish(attackEvent);
        }
    }
}