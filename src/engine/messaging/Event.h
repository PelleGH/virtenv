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
    bool isDirectHit = false;
    Entity directTarget;
};

struct DeathEvent {
    Entity entity;
};

struct EnemyKilledEvent {
    std::string enemyType; // e.g., "slime", "goblin"
};
struct GameplayEvent {
    std::string type;
    std::string targetId;
    int amount = 1;
};

struct DialogueStartEvent {
    std::string dialogueId;
};

struct OverlapEvent {
    Entity entityA;
    Entity entityB;
    bool hitWall = false;
};

struct PickupEvent {
    Entity player;
    Entity groundEntity; // The cube on the ground
    std::string itemId;  // The data inside it
};

struct EquipEvent {
    Entity player;
    std::string itemId;  // The ID we want to equip from our inventory
};