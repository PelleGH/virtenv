#pragma once

#include <string>
#include "../ecs/Entity.h"

struct SceneTransitionEvent
{
    std::string targetScene;
    std::string targetSpawn;
};

struct AttackEvent
{
    Entity attacker;
    float attackRange;
    int damage;
};

struct DeathEvent {
    Entity entity;
};