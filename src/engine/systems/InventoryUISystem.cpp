#include "InventoryUISystem.h"
#include "../scene/Scene.h"
#include "../ecs/Components.h"
#include "../resource/ResourceManager.h"
#include "raylib.h"

void InventoryUISystem::render(Scene& scene, ResourceManager& resManager) {
    ComponentStorage& components = scene.getComponentStorage();

    // Find the player
    for (auto& [entity, input] : components.GetComponents<PlayerInput>()) {
        
        // --- 1. DRAW INVENTORY ---
        if (components.HasComponent<Inventory>(entity)) {
            auto& inv = components.GetComponent<Inventory>(entity);
            
            // Draw a semi-transparent background panel
            DrawRectangle(10, 10, 300, 300, Fade(BLACK, 0.7f));
            DrawText("INVENTORY (Press 1-9 to Equip)", 20, 20, 16, WHITE);
            DrawLine(20, 45, 290, 45, GRAY);

            int startY = 60;
            for (size_t i = 0; i < inv.items.size(); i++) {
                // Get the actual display name from the resource manager
                ItemData item = resManager.getItem(inv.items[i]);
                std::string displayText = std::to_string(i + 1) + ". " + item.name;
                
                DrawText(displayText.c_str(), 20, startY + (i * 30), 20, LIGHTGRAY);
            }
        }

        // --- 2. DRAW LOADOUT ---
        if (components.HasComponent<Loadout>(entity)) {
            auto& loadout = components.GetComponent<Loadout>(entity);
            
            DrawRectangle(10, 320, 300, 120, Fade(DARKBLUE, 0.7f));
            DrawText("EQUIPPED", 20, 330, 16, WHITE);
            DrawLine(20, 355, 290, 355, GRAY);

            // Weapon
            std::string weaponName = loadout.weaponId.empty() ? "None" : resManager.getItem(loadout.weaponId).name;
            DrawText(("Weapon: " + weaponName).c_str(), 20, 370, 20, ORANGE);

            // Armor
            std::string armorName = loadout.armorId.empty() ? "None" : resManager.getItem(loadout.armorId).name;
            DrawText(("Armor:  " + armorName).c_str(), 20, 400, 20, SKYBLUE);
        }
    }
}