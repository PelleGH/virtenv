/*
 * How to add a new interactable object (Doors, NPCs, Items etc.)
 *
 * 1. Open the scene file (.json) where the object should be placed.
 * 2. Locate the "entities" array, create a new block inside it, and add 
 *    a TransformComponent and an Interactable component. 
 * 3. If the added action is new (does not exist yet), you need to register 
 *    it in C++ inside Engine::init() under "actionHandlers".
 *
 * Examples:
 * - Look at the bottom of "room_01.json" for steps 1 and 2.
 * - Look at the bottom of the Engine::init() function in Engine.cpp for step 3.
 */

 
#include "InteractionSystem.h"
#include "engine/ecs/Components.h"
#include "engine/messaging/Event.h"

void InteractionSystem::update(Scene& scene, EventBus& eventBus){
    ComponentStorage& componentStorage = scene.getComponentStorage();
    auto& player = componentStorage.GetComponents<PlayerInput>();

    // Entities with player input
    for (auto& [playerEntity, input] : player){

        //If player did not pressed "E"
        if (!input.interact){
            continue;
        }

        //Ensures player position in the worls
        if (!componentStorage.HasComponent<TransformComponent>(playerEntity)){
            continue;
        }

        TransformComponent& playerTransform = componentStorage.GetComponent<TransformComponent>(playerEntity);

        //Get interactables objects within the scene
        auto& interactables = componentStorage.GetComponents<Interactable>();

        // To handle the closest object / entity within the radius
        Entity closestEntity;
        Interactable* closestInteractable = nullptr;
        float closestDist = std::numeric_limits<float>::max();
        bool foundEntity = false;
        
        // Go through all interactables in the room
        for (auto& [targetEntity, interactable] : interactables){

            //Ensures that the object / entity have a position
            if (!componentStorage.HasComponent<TransformComponent>(targetEntity)){
                continue;
            }

            TransformComponent& targetTransform = componentStorage.GetComponent<TransformComponent>(targetEntity);

            // Calculate distance
            float dx = playerTransform.x - targetTransform.x;
            float dz = playerTransform.z - targetTransform.z;
            float dist = (dx * dx) + (dz * dz);
            float radiusSq = interactable.interactionRadius * interactable.interactionRadius;

            // Was player close enough when pressing "E"? Is the current object closer than the previous object?
            if (dist < radiusSq && dist < closestDist){

                closestDist = dist;
                closestEntity = targetEntity;
                closestInteractable = &interactable;
                foundEntity = true;
            }
        }

        // Event for the closest object
        if (foundEntity && closestInteractable != nullptr){

            for (const auto& action : closestInteractable -> actions){
                ActionEvent actionEvent;
                actionEvent.initiator = playerEntity;
                actionEvent.targetEntity = closestEntity;
                actionEvent.actionType = action.type;
                actionEvent.targetData = action.target;

                eventBus.publish(actionEvent);
            }
            
        }
        input.interact = false;
        return;
    }
}