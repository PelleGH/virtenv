#pragma once
#include <string>
#include <vector>
#include "Entity.h"
#include "raylib.h"

// CORE ENGINE COMPONENTS

struct TransformComponent {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    float previousX = 0.0f;
    float previousY = 0.0f;
    float previousZ = 0.0f;

    float width = 1.0f;
    float height = 1.0f;
    float depth = 1.0f;
};

struct Renderer {
    std::string textureID; // e.g., "player_idle", "door_open"
    std::string modelID;

    float width = 1.0f;
    float height = 1.0f;
    float depth = 1.0f;
    Color color = RED;

    int zIndex = 0;
};

struct Collider {
    float width = 1.0f;
    float height = 1.0f;
    float depth = 1.0f;
    bool isTrigger = false;
    bool enabled = true;
};

struct PlayerInput {
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
    bool attack = false;
    bool interact = false;
};

struct SpawnType {
    std::string entityToSpawn; // e.g., "player", "npc_merchant", "enemy_goblin"
    bool hasSpawned = false;   // Used to ensure we only spawn it once on startup
    int skinChoice = 1;
};

struct Condition
{
    std::string type;
    std::string id;
};

struct ConditionalBlocker
{
    std::vector<Condition> conditions;
};
// COMBAT & STATS

struct Health {
    int current = 100;
    int max = 100;
};

struct Attack {
    float range = 10.0f;
    int damage = 10;
    float cooldown = 1.0f;
    float timeSinceLastAttack = 0.0f; 
};

struct Loadout {
    Entity weaponSlot{}; // ID of the equipped weapon entity
    Entity armorSlot{};  // ID of the equipped armor entity
};


// INVENTORY & BUFFS (Pure Data)

enum class BuffType {
    None,
    Heal,
    SpeedBoost,
    DamageUp
};

struct ItemData {
    std::string name;       // e.g., "Minor Health Potion"
    BuffType type = BuffType::None;          
    float amount = 0.0f;    
    float duration = 0.0f;  // 0.0 for instant use, > 0.0 for timed buffs
};

struct Inventory {
    std::vector<ItemData> items;
    int maxSlots = 10;
};

struct ActiveBuff {
    BuffType type;
    float amount;
    float timeRemaining;
};

struct BuffContainer {
    std::vector<ActiveBuff> activeBuffs; 
};


// INTERACTABLES (Split by behavior)

// Attached to an entity on the ground.  Holds the pure data that will be moved into the Inventory.
struct Pickup {
    ItemData item; 
};

struct SceneTransition {
    std::string targetScene; // e.g., "tavern_interior"
    float spawnX = 0.0f;     // Where to place the player after loading
    float spawnY = 0.0f;
    float spawnZ = 0.0f;
};

struct DialogueSource {
    std::string dialogueSetId;
};

// A Tag Component. The Combat System checks if an entity has this, and if its Health drops to 0, it drops loot and gets destroyed.
struct Breakable {
    std::string dropLootID = ""; 
};