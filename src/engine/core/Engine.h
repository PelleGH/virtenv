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
#include "../messaging/Event.h"
#include "../systems/ConditionManager.h"
#include "../systems/ConditionSystem.h"
#include "../systems/DialogueManager.h"
#include "../managers/QuestManager.h"
#include "../ecs/Components.h"
#include "../systems/CombatSystem.h"
#include "../systems/AISystem.h"

#include "../resource/ResourceManager.h"

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
    CombatSystem combatSystem;
    AISystem aiSystem;
    ResourceManager resourceManager;

    void debugNPCInteraction();
    
    void update(float dt);
    void render();
};