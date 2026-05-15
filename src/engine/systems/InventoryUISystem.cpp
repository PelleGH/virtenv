#include "InventoryUISystem.h"
#include "../scene/Scene.h"
#include "../ecs/Components.h"
#include "../resource/ResourceManager.h"
#include "raylib.h"

void InventoryUISystem::render(Scene& scene, ResourceManager& resManager) {
    ComponentStorage& components = scene.getComponentStorage();

    // Find the player
    for (auto& [entity, input] : components.GetComponents<PlayerInput>()) {
        
        int startX = 20; 
        int startY = 60;                     

        int panelWidth = 270;

        // draw inventory
        if (components.HasComponent<Inventory>(entity)) {
            auto& inv = components.GetComponent<Inventory>(entity);
            
            DrawRectangle(startX, startY, panelWidth, 300, Fade(BLACK, 0.7f));
            DrawText("INVENTORY (Press 1-9 to Equip)", startX + 10, startY + 10, 16, WHITE);
            DrawLine(startX + 10, startY + 35, startX + panelWidth - 10, startY + 35, GRAY);

            int textY = startY + 50;
            for (size_t i = 0; i < inv.items.size(); i++) {
                ItemData item = resManager.getItem(inv.items[i]);
                std::string displayText = std::to_string(i + 1) + ". " + item.name;
                
                DrawText(displayText.c_str(), startX + 10, textY + (i * 30), 20, LIGHTGRAY);
            }
        }

        // draw loadout
        if (components.HasComponent<Loadout>(entity)) {
            auto& loadout = components.GetComponent<Loadout>(entity);
            
            // Draw this panel directly below the inventory panel
            int loadoutY = startY + 310; 

            DrawRectangle(startX, loadoutY, panelWidth, 120, Fade(DARKBLUE, 0.7f));
            DrawText("EQUIPPED", startX + 10, loadoutY + 10, 16, WHITE);
            DrawLine(startX + 10, loadoutY + 35, startX + panelWidth - 10, loadoutY + 35, GRAY);

            // Weapon
            std::string weaponName = loadout.weaponId.empty() ? "None" : resManager.getItem(loadout.weaponId).name;
            DrawText(("Weapon: " + weaponName).c_str(), startX + 10, loadoutY + 50, 20, ORANGE);

            // Armor
            std::string armorName = loadout.armorId.empty() ? "None" : resManager.getItem(loadout.armorId).name;
            DrawText(("Armor:  " + armorName).c_str(), startX + 10, loadoutY + 80, 20, SKYBLUE);
        }
    }
}