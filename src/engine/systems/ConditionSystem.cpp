#include "ConditionSystem.h"

#include "../scene/Scene.h"
#include "../ecs/Components.h"
#include "../systems/ConditionManager.h"

void ConditionSystem::update(Scene& scene, ConditionManager& conditionManager)
{
    auto& components = scene.getComponentStorage();
    auto& blockers = components.GetComponents<ConditionalBlocker>();

    for (auto& [entity, blocker] : blockers)
    {
        if (!components.HasComponent<Collider>(entity))
            continue;

        auto& collider = components.GetComponent<Collider>(entity);

        bool unlocked =
            conditionManager.conditionsMet(blocker.conditions);

        collider.enabled = !unlocked;
    }
}