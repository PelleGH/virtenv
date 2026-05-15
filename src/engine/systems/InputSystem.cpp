#include "InputSystem.h"
#include "../scene/Scene.h"
#include "../ecs/Components.h"
#include "../messaging/EventBus.h"
#include "raylib.h"

void InputSystem::update(Scene& scene, EventBus& eventBus)
{
    ComponentStorage& components = scene.getComponentStorage();
    auto& playerInputs = components.GetComponents<PlayerInput>();

    // Get time since last frame to update our cooldowns
    float dt = GetFrameTime();

    for (auto& [entity, input] : playerInputs)
    {
        input.up = IsKeyDown(KEY_W);
        input.down = IsKeyDown(KEY_S);
        input.left = IsKeyDown(KEY_A);
        input.right = IsKeyDown(KEY_D);

        input.attack = IsKeyPressed(KEY_SPACE);
        input.interact = IsKeyPressed(KEY_E);

        // --- TRANSLATE INPUT TO VELOCITY ---
        if (components.HasComponent<Velocity>(entity)) {
            auto& vel = components.GetComponent<Velocity>(entity);
            float moveX = 0.0f;
            float moveZ = 0.0f;

            if (input.right) moveX += 1.0f;
            if (input.left)  moveX -= 1.0f;
            if (input.up)    moveZ -= 1.0f;
            if (input.down)  moveZ += 1.0f;

            float length = std::sqrt(moveX * moveX + moveZ * moveZ);
            if (length > 0.0f) {
                moveX /= length;
                moveZ /= length;
            }

            vel.x = moveX;
            vel.z = moveZ;
        }

        // --- DYNAMIC ATTACK & COOLDOWN LOGIC ---
        if (components.HasComponent<Attack>(entity))
        {
            auto& attackStats = components.GetComponent<Attack>(entity);
            
            // 1. Always increase the timer by the time passed this frame
            attackStats.timeSinceLastAttack += dt;

            // 2. Check if player pressed attack AND the cooldown is finished
            if (input.attack && attackStats.timeSinceLastAttack >= attackStats.cooldown)
            {
                // Reset the timer since we are attacking right now
                attackStats.timeSinceLastAttack = 0.0f;

                // Create and publish the event using the component's data
                AttackEvent attackEvent;
                attackEvent.attacker = entity;
                attackEvent.attackRange = attackStats.range;
                attackEvent.damage = attackStats.damage;
                
                eventBus.publish(attackEvent);
            }
        }

        // INVENTORY EQUIP
        if (IsKeyPressed(KEY_ONE) && components.HasComponent<Inventory>(entity)) {
            auto& inv = components.GetComponent<Inventory>(entity);
            if (!inv.items.empty()) {
                EquipEvent equip;
                equip.player = entity;
                equip.itemId = inv.items[0];
                eventBus.publish(equip);
            }
        }
    }
}