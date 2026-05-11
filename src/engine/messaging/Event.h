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

struct EnemyKilledEvent {
    std::string enemyType; // e.g., "slime", "goblin"
};
struct QuestProgressEvent {
    std::string eventType;
    std::string targetId;
};

struct DialogueStartEvent {
    std::string dialogueId;
};