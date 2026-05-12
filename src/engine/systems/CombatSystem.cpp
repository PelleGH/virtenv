#include "CombatSystem.h"
#include "../ecs/Components.h"

#include <iostream>

void CombatSystem::initialize(Scene& scene, EventBus& eventBus)
{
    m_scene = &scene;
    m_eventBus = &eventBus;

    // Subscribe to the AttackEvent
    eventBus.subscribe<AttackEvent>([this](const AttackEvent& e) {
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
        if (entity == event.attacker) continue;
        if (!components.HasComponent<TransformComponent>(entity)) continue;

        bool isTargetPlayer = components.HasComponent<PlayerInput>(entity);
        if (isAttackerPlayer == isTargetPlayer) continue; 

        bool hitConfirmed = false;

        // DIRECT HIT (from a projectile)
        if (event.isDirectHit) {
            if (entity == event.directTarget) {
                hitConfirmed = true;
            }
        } 
        // SPHERICAL HIT (from melee)
        else {
            auto& targetTransform = components.GetComponent<TransformComponent>(entity);
            float dx = attackerTransform.x - targetTransform.x;
            float dy = attackerTransform.y - targetTransform.y;
            float dz = attackerTransform.z - targetTransform.z;

            float distanceSquared = (dx * dx) + (dy * dy) + (dz * dz);
            float rangeSquared = event.attackRange * event.attackRange;

            if (distanceSquared <= rangeSquared) {
                hitConfirmed = true;
            }
        }

        // APPLY DAMAGE
        if (hitConfirmed && health.current > 0)
        {
            health.current -= event.damage; 
            std::cout << "Entity " << entity.id << " took " << event.damage 
                      << " damage! HP remaining: " << health.current << '\n';

            if (health.current <= 0)
            {
                if (!components.HasComponent<PlayerInput>(entity)) {
                    std::string enemyType = "enemy";
                    if (components.HasComponent<Attack>(entity)) {
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