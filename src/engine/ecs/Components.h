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
    std::string textureID;  // Used for texture, e.g. "player_tex1"
    std::string modelID;    // Used for models, e.g. "player_skin1"

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
    int skinChoice = 1;        // Default skin if nothing else is written in json
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
    int defense = 0;
};

struct Attack {
    float range = 10.0f;
    int damage = 10;
    float cooldown = 1.0f;
    float timeSinceLastAttack = 0.0f; 

    std::string enemyType = "enemy";

    bool isRanged = false;
    float projectileSpeed = 5.0f;
};

enum class AIType {
    Melee,
    Ranged
};

struct AIController {
    AIType type = AIType::Melee;

    float aggroRange = 6.0f;
    float leashRange = 9.0f;

    float preferredRange = 5.0f; // ranged only
    float minimumRange = 2.0f;   // ranged only

    float sidestepTimer = 0.0f;
    int sidestepDirection = 0; // -1 = left, 1 = right, 0 = none
};

struct Velocity {
    float x = 0.0f;
    float z = 0.0f;
    float speed = 1.0f;
};

struct Projectile {
    int damage = 10;
    float timeToLive = 5.0f; //Destroy projectile after 5 seconds
    Entity owner; //to prevent hitting themselves
};


// INVENTORY & BUFFS (Pure Data)

enum class BuffType {
    None,
    Heal,
    SpeedBoost,
    DamageUp
};

enum class EquipSlot {
    None,
    Weapon,
    Armor,
    Consumable
};

struct ItemData {
    std::string id;         
    std::string name;       
    EquipSlot slot = EquipSlot::None;
    
    //Stats it gives when equipped
    int damageBonus = 0;
    int healthBonus = 0;
    int defenseBonus = 0;
};

struct Inventory {
    std::vector<std::string> items; // Holds string IDs like "iron_sword"
    int maxSlots = 10;
};

struct Loadout {
    std::string weaponId = ""; 
    std::string armorId = "";
};

struct Pickup {
    // Empty tag! We don't need data here because the InteractionSystem handles it
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

struct ActionDefinition{
    std::string type;       //What to do e.g. "StartDialogue"
    std::string target;     //Goal e.g. "old_man"
};

struct Interactable{   
    float interactionRadius = 1.0f;             //How close the player need to be to the object
    std::vector<ActionDefinition> actions;      //List with all event that can be triggers      
};