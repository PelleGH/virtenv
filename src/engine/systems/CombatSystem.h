#pragma once

#include "../scene/Scene.h"
#include "../messaging/EventBus.h"

class CombatSystem
{
public:
    // Bind the system to the scene and subscribe to the EventBus
    void initialize(Scene& scene, EventBus& eventBus);

private:
    // The callback triggered by the EventBus
    void onAttack(const AttackEvent& event);

    Scene* m_scene = nullptr;
};