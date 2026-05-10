#pragma once
#include "raylib.h"
#include "engine/scene/SceneManager.h"
#include "../systems/RenderSystem.h"
#include "../systems/InputSystem.h"
#include "../systems/MovementSystem.h"
#include "../systems/CollisionSystem.h"
#include "../systems/CameraSystem.h"
#include "../systems/TriggerSystem.h"
#include "../messaging/EventBus.h"
#include "../systems/ConditionManager.h"
#include "../systems/ConditionSystem.h"
#include "../systems/DialogueManager.h"
#include "../managers/QuestManager.h"
#include "../ecs/Components.h"

class Engine
{
public:
    bool init();
    void run();
    void shutdown();

private:
    bool running = false;
    InputSystem inputSystem;
    SceneManager sceneManager;
    RenderSystem renderSystem;
    MovementSystem movementSystem;
    CollisionSystem collisionSystem;
    CameraSystem cameraSystem;
    TriggerSystem triggerSystem;
    EventBus eventBus;
    Camera3D camera;
    ConditionManager conditionManager;
    ConditionSystem conditionalSystem;
    QuestManager questManager;
    DialogueManager dialogueManager;
    void debugNPCInteraction();
    
    void update(float dt);
    void render();
};