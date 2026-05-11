#include "CombatSystem.h"
#include "../ecs/Components.h"

#include <iostream>

void CombatSystem::initialize(Scene& scene, EventBus& eventBus)
{
    m_scene = &scene;
    m_eventBus = &eventBus;

    // Subscribe to the AttackEvent
    eventBus.subscribe([this](const AttackEvent& e) {
        this->onAttack(e);
    });
}

void CombatSystem::onAttack(const AttackEvent& event)
{
    if (!m_scene) return;

    ComponentStorage& components = m_scene->getComponentStorage();

    // Ensure the attacker actually has a transform
    if (!components.HasComponent<TransformComponent>(event.attacker)) return;
    auto& attackerTransform = components.GetComponent<TransformComponent>(event.attacker);

    // Check if the attacker is the player
    bool isAttackerPlayer = components.HasComponent<PlayerInput>(event.attacker);

    // Get all entities with Health components
    auto& healthComponents = components.GetComponents<Health>();

    for (auto& [entity, health] : healthComponents)
    {
        // Don't let the attacker damage themselves
        if (entity == event.attacker) continue;

        // Ensure the target has a position in the world
        if (!components.HasComponent<TransformComponent>(entity)) continue;

        // Check if the target is the player
        bool isTargetPlayer = components.HasComponent<PlayerInput>(entity);

        // --- FRIENDLY FIRE FILTER ---
        // If a player attacks a player, or an enemy attacks an enemy, skip it!
        if (isAttackerPlayer == isTargetPlayer) continue; 

        auto& targetTransform = components.GetComponent<TransformComponent>(entity);

        // 3D Distance check
        float dx = attackerTransform.x - targetTransform.x;
        float dy = attackerTransform.y - targetTransform.y;
        float dz = attackerTransform.z - targetTransform.z;

        float distanceSquared = (dx * dx) + (dy * dy) + (dz * dz);
        float rangeSquared = event.attackRange * event.attackRange;

        // Apply damage if within range
        if (distanceSquared <= rangeSquared)
        {
            // Only deal damage if they are actually alive
            if (health.current > 0) 
            {
                health.current -= event.damage; 
                std::cout << "Entity " << entity.id << " took " << event.damage 
                          << " damage! HP remaining: " << health.current << '\n';

                // If this hit killed them, publish the DeathEvent!
                if (health.current <= 0)
                {
                    if (!components.HasComponent<PlayerInput>(entity))
                    {
                        std::string enemyType = "enemy";

                        if (components.HasComponent<Attack>(entity))
                        {
                            enemyType = components.GetComponent<Attack>(entity).enemyType;
                        }

                        EnemyKilledEvent killedEvent;
                        killedEvent.enemyType = enemyType;
                        m_eventBus->publish(killedEvent);
                    }

                    DeathEvent deathEvent;
                    deathEvent.entity = entity;
                    m_eventBus->publish(deathEvent);
                }
            }
        }
    }
}