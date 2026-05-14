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

    eventBus.subscribe<PickupEvent>([this](const PickupEvent& e) { this->onPickup(e); });
    eventBus.subscribe<EquipEvent>([this](const EquipEvent& e) { this->onEquip(e); });
}

void InventorySystem::onPickup(const PickupEvent& event) {
    ComponentStorage& components = m_scene->getComponentStorage();
    if (!components.HasComponent<Inventory>(event.player)) return;
    
    auto& inv = components.GetComponent<Inventory>(event.player);
    if (inv.items.size() >= inv.maxSlots) return;

    // 1. Add string ID to inventory
    inv.items.push_back(event.itemId);

    // 2. Destroy the physical entity on the ground
    m_scene->queueEntityDestruction(event.groundEntity);
    
    std::cout << "Picked up " << event.itemId << "!\n";
}

void InventorySystem::onEquip(const EquipEvent& event) {
    ComponentStorage& components = m_scene->getComponentStorage();
    if (!components.HasComponent<Loadout>(event.player) || !components.HasComponent<Inventory>(event.player)) return;

    auto& loadout = components.GetComponent<Loadout>(event.player);
    auto& inv = components.GetComponent<Inventory>(event.player);

    // Check if player actually has the item
    auto it = std::find(inv.items.begin(), inv.items.end(), event.itemId);
    if (it == inv.items.end()) return; 

    // Get the pure data from our database
    ItemData itemData = m_resManager->getItem(event.itemId);

    // Swap logic
    std::string oldItem = "";

    if (itemData.slot == EquipSlot::Weapon) {
        oldItem = loadout.weaponId;
        loadout.weaponId = event.itemId;

        // Apply stats!
        if (components.HasComponent<Attack>(event.player)) {
            auto& attack = components.GetComponent<Attack>(event.player);
            // Example: Base damage of 10 + weapon damage
            attack.damage = 10 + itemData.damageBonus; 
        }
        std::cout << "Equipped Weapon: " << itemData.name << "\n";
    } 
    else if (itemData.slot == EquipSlot::Armor) {
        oldItem = loadout.armorId;
        loadout.armorId = event.itemId;
        // Apply health/defense stats here...
        std::cout << "Equipped Armor: " << itemData.name << "\n";
    }

    // Remove new item from inventory, put old item back in (if it existed)
    inv.items.erase(it);
    if (!oldItem.empty()) {
        inv.items.push_back(oldItem);
    }
}