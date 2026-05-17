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

struct ActionEvent{
    Entity initiator;           //Who pressed the button
    Entity targetEntity;        //What is pressed
    std::string actionType;     //What to do e.g. "StartDialogue"
    std::string targetData;     //Goal e.g. "old_man"
};

struct EquipEvent {
    Entity player;
    std::string itemId;  
};

struct DropItemEvent {
    Entity player;
    int inventoryIndex;
};

struct SpawnItemDropEvent {
    float x, y, z;
    std::string itemId;
};