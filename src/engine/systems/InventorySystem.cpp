#include "InventorySystem.h"
#include "../scene/Scene.h"
#include "../ecs/Components.h"
#include "../messaging/EventBus.h"
#include "../resource/ResourceManager.h"
#include <iostream>
#include <algorithm>

void InventorySystem::initialize(Scene& scene, EventBus& eventBus, ResourceManager& resManager) {
    m_scene = &scene;
    m_eventBus = &eventBus;
    m_resManager = &resManager;

    // Listen for world interactions AND equipment hotkeys
    eventBus.subscribe<ActionEvent>([this](const ActionEvent& e) { this->onAction(e); });
    eventBus.subscribe<EquipEvent>([this](const EquipEvent& e) { this->onEquip(e); });
    eventBus.subscribe<DropItemEvent>([this](const DropItemEvent& e) { this->onDropItem(e); });
}

void InventorySystem::onAction(const ActionEvent& event) {
    // Only intercept "PickupItem" interactions
    if (event.actionType != "PickupItem") return;

    ComponentStorage& components = m_scene->getComponentStorage();
    if (!components.HasComponent<Inventory>(event.initiator)) return;
    
    auto& inv = components.GetComponent<Inventory>(event.initiator);
    if (inv.items.size() >= inv.maxSlots) {
        std::cout << "Inventory is full!\n";
        return;
    }

    // 1. Add string ID (targetData) to player inventory
    inv.items.push_back(event.targetData);

    // 2. Destroy the physical cube on the ground
    m_scene->queueEntityDestruction(event.targetEntity);
    
    std::cout << "Picked up " << event.targetData << "!\n";
}

void InventorySystem::onEquip(const EquipEvent& event) {
    ComponentStorage& components = m_scene->getComponentStorage();
    if (!components.HasComponent<Loadout>(event.player) || !components.HasComponent<Inventory>(event.player)) return;

    auto& loadout = components.GetComponent<Loadout>(event.player);
    auto& inv = components.GetComponent<Inventory>(event.player);

    // Check if player actually has the item
    auto it = std::find(inv.items.begin(), inv.items.end(), event.itemId);
    if (it == inv.items.end()) return; 

    // Get the pure data from the database
    ItemData itemData = m_resManager->getItem(event.itemId);
    std::string oldItem = "";

    if (itemData.slot == EquipSlot::Weapon) {
        oldItem = loadout.weaponId;
        loadout.weaponId = event.itemId;

        // Apply Base damage + weapon damage
        if (components.HasComponent<Attack>(event.player)) {
            components.GetComponent<Attack>(event.player).damage = 10 + itemData.damageBonus; 
        }
        std::cout << "Equipped Weapon: " << itemData.name << "\n";
    } 
    else if (itemData.slot == EquipSlot::Armor) 
    {
        oldItem = loadout.armorId;
        loadout.armorId = event.itemId;

        if (components.HasComponent<Health>(event.player)) 
        {
            auto& health = components.GetComponent<Health>(event.player);
            
            int oldMax = health.max;
            
            // Set new Max HP (Base 100 + Armor Bonus)
            health.max = 100 + itemData.healthBonus; 
            
            // Adjust current health based on the difference
            health.current += (health.max - oldMax); 
            
            // Don't let them have more than max HP
            if (health.current > health.max) 
            {
                health.current = health.max;
            }
            
            // Don't let equipping a worse armor kill them! Cap at 1 HP.
            if (health.current < 1) 
            { 
                health.current = 1;
            }

            // Apply the flat damage reduction defense stat
            health.defense = itemData.defenseBonus; 
        }
        
        std::cout << "Equipped Armor: " << itemData.name << "\n";
    }
    else if (itemData.slot == EquipSlot::Consumable) 
    {
        if (components.HasComponent<Health>(event.player)) 
        {
            auto& health = components.GetComponent<Health>(event.player);
            
            // Check if player is already at max health
            if (health.current >= health.max) {
                std::cout << "Health is already full!\n";
                return; // Cancel the consumption
            }

            // Heal the player
            health.current += itemData.healthBonus;
            if (health.current > health.max) {
                health.current = health.max;
            }
            
            std::cout << "Drank " << itemData.name << "! Healed for " << itemData.healthBonus << " HP.\n";
            std::cout << "Current HP: " << health.current << "/" << health.max << "\n";
        }
        
        // Remove the potion from the inventory completely (no old item to swap back in!)
        inv.items.erase(it);
        return; // Return immediately so it doesn't run the swap logic at the bottom of the function
    }

    // Remove new item from inventory, put old item back in (if it existed)
    inv.items.erase(it);
    if (!oldItem.empty()) inv.items.push_back(oldItem);
}

void InventorySystem::onDropItem(const DropItemEvent& event) {
    ComponentStorage& components = m_scene->getComponentStorage();
    
    if (components.HasComponent<Inventory>(event.player) && components.HasComponent<TransformComponent>(event.player)) {
        auto& inv = components.GetComponent<Inventory>(event.player);
        auto& playerTransform = components.GetComponent<TransformComponent>(event.player);

        if (event.inventoryIndex >= 0 && event.inventoryIndex < inv.items.size()) {
            std::string itemToDrop = inv.items[event.inventoryIndex];
            
            // 1. Remove it from the UI/Bag
            inv.items.erase(inv.items.begin() + event.inventoryIndex);
            
            // 2. Fire an event to request the drop!
            SpawnItemDropEvent spawnRequest;
            spawnRequest.x = playerTransform.x;
            spawnRequest.y = playerTransform.y;
            spawnRequest.z = playerTransform.z - 1.0f;
            spawnRequest.itemId = itemToDrop;
            
            m_eventBus->publish(spawnRequest);
            
            std::cout << "Dropped " << itemToDrop << " on the floor!\n";
        }
    }
}