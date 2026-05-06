#pragma once

#include <string>
#include "../ecs/Entity.h"

struct SceneTransitionEvent
{
    std::string targetScene;
};

struct AttackEvent
{
    Entity attacker;
    float attackRange;
    int damage;
};