#include "InventoryUISystem.h"
#include "../scene/Scene.h"
#include "../ecs/Components.h"
#include "../resource/ResourceManager.h"
#include "raylib.h"

void InventoryUISystem::render(Scene& scene, ResourceManager& resManager) {
    ComponentStorage& components = scene.getComponentStorage();

    for (auto& [entity, input] : components.GetComponents<PlayerInput>()) {
        
        // DRAW INVENTORY
        if (components.HasComponent<Inventory>(entity)) {
            auto& inv = components.GetComponent<Inventory>(entity);
            
            DrawRectangle(10, 50, 380, 300, Fade(BLACK, 0.7f));
            DrawText("INVENTORY (Press 1-9 to Equip)", 20, 60, 16, WHITE);
            DrawLine(20, 85, 370, 85, GRAY);

            int startY = 100;
            for (size_t i = 0; i < inv.items.size(); i++) {
                ItemData item = resManager.getItem(inv.items[i]);
                std::string displayText = std::to_string(i + 1) + ". " + item.name;
                
                // Draw the item name
                DrawText(displayText.c_str(), 20, startY + (i * 30), 20, LIGHTGRAY);

                //BUILD THE STATS
                std::string stats = "";
                if (item.slot == EquipSlot::Weapon) {
                    stats = "(+" + std::to_string(item.damageBonus) + " Dmg)";
                } 
                else if (item.slot == EquipSlot::Armor) {
                    stats = "(+" + std::to_string(item.defenseBonus) + " Def, +" + std::to_string(item.healthBonus) + " HP)";
                }
                else if (item.slot == EquipSlot::Consumable) {
                    stats = "(Heals " + std::to_string(item.healthBonus) + ")";
                }

                // Calculate where the name ends so we can draw the stats right next to it
                int nameWidth = MeasureText(displayText.c_str(), 20);
                
                // Draw the stats in a darker gray and slightly smaller font
                DrawText(stats.c_str(), 20 + nameWidth + 10, startY + (i * 30) + 2, 16, GRAY);
            }
        }

        // DRAW LOADOUT
        if (components.HasComponent<Loadout>(entity)) {
            auto& loadout = components.GetComponent<Loadout>(entity);
            
            DrawRectangle(10, 360, 380, 120, Fade(DARKBLUE, 0.7f));
            DrawText("EQUIPPED", 20, 370, 16, WHITE);
            DrawLine(20, 395, 370, 395, GRAY);

            // Weapon
            std::string weaponStr = "Weapon: None";
            if (!loadout.weaponId.empty()) {
                ItemData w = resManager.getItem(loadout.weaponId);
                weaponStr = "Weapon: " + w.name + " (+" + std::to_string(w.damageBonus) + " Dmg)";
            }
            DrawText(weaponStr.c_str(), 20, 410, 20, ORANGE);

            // Armor
            std::string armorStr = "Armor:  None";
            if (!loadout.armorId.empty()) {
                ItemData a = resManager.getItem(loadout.armorId);
                armorStr = "Armor:  " + a.name + " (+" + std::to_string(a.defenseBonus) + " Def)";
            }
            DrawText(armorStr.c_str(), 20, 440, 20, SKYBLUE);
        }
    }
}