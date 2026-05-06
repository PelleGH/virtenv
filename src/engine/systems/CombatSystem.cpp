// src/engine/systems/CombatSystem.cpp
#include "CombatSystem.h"
#include "../ecs/Components.h"

#include <iostream>
#include <cmath>

void CombatSystem::initialize(Scene& scene, EventBus& eventBus)
{
    m_scene = &scene;

    // Subscribe to the AttackEvent using a lambda to bind the member function
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

    // Get all entities with Health components
    auto& healthComponents = components.GetComponents<Health>();

    for (auto& [entity, health] : healthComponents)
    {
        // Don't let the attacker damage themselves
        if (entity == event.attacker) continue;

        // Ensure the target has a position in the world
        if (!components.HasComponent<TransformComponent>(entity)) continue;

        auto& targetTransform = components.GetComponent<TransformComponent>(entity);

        // 3D Distance check (avoiding heavy std::sqrt)
        float dx = attackerTransform.x - targetTransform.x;
        float dy = attackerTransform.y - targetTransform.y;
        float dz = attackerTransform.z - targetTransform.z;

        float distanceSquared = (dx * dx) + (dy * dy) + (dz * dz);
        float rangeSquared = event.attackRange * event.attackRange;

        // Apply damage if within range
        if (distanceSquared <= rangeSquared)
        {
            // Note: Adjust "current" below to match whatever you named the HP variable in your Health struct
            health.current -= event.damage; 
            
            // Update the damage print statement:
            std::cout << "Entity " << entity.id << " took " << event.damage 
                    << " damage! HP remaining: " << health.current << '\n';

            if (health.current <= 0)
            {
                // Update the death print statement:
                std::cout << "Entity " << entity.id << " died!\n";
                // Optional: m_scene->getEntityManager().destroyEntity(entity);
            }
        }
    }
}