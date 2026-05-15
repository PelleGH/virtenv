#include "Engine.h"
#include "engine/scene/Scene.h"

#include <chrono>
#include <iostream>

bool Engine::init()
{
    InitWindow(1280, 720, "Virtenv");
    SetTargetFPS(60);

    //Reads and creates the models and texture for the world
    resourceManager.LoadFromManifest("src/engine/assets/assets.json");
    sceneManager.init(resourceManager);

    cameraSystem.init(camera);

    questManager.setConditionManager(&conditionManager);
    questManager.loadQuests("assets/quests.json");
    dialogueManager.setQuestManager(&questManager);

    eventBus.subscribe<SceneTransitionEvent>([this](const SceneTransitionEvent& event)
    {
        sceneManager.requestSceneChange(
            "assets/scenes/" + event.targetScene + ".json",
            event.targetSpawn
        );
    });

    eventBus.subscribe<DeathEvent>([this](const DeathEvent& event)
    {
        Scene& currentScene = sceneManager.getCurrentScene();
        ComponentStorage& components = currentScene.getComponentStorage();

        // Check if the entity that died is the player
        if (components.HasComponent<PlayerInput>(event.entity))
        {
            std::cout << "Player died! Reloading scene...\n";
            
            // Get the current scene's name (e.g., "room_01")
            std::string sceneName = currentScene.getData().name;
            
            // Queue a reload of the current scene, spawning at the default location
            sceneManager.requestPlayerRespawn();    
        }
        else
        {
            // It's just a normal enemy/cube, destroy it
            currentScene.queueEntityDestruction(event.entity);
        }
    });
    eventBus.subscribe<EnemyKilledEvent>([this](const EnemyKilledEvent& event)
    {
        GameplayEvent gameplayEvent;
        gameplayEvent.type = "enemy_killed";
        gameplayEvent.targetId = event.enemyType;
        gameplayEvent.amount = 1;

        eventBus.publish(gameplayEvent);
    });

    eventBus.subscribe<GameplayEvent>([this](const GameplayEvent& event)
    {
        questManager.onEvent(event.type, event.targetId, event.amount);
    });

    // Interaction type
    actionHandlers["StartDialogue"] = [this](const std::string& data){
        this -> dialogueManager.startDialogue(data);
    };

    eventBus.subscribe<ActionEvent>([this](const ActionEvent& actionEvent) {
        this->onActionEvent(actionEvent);
    });
    
    running = true;

    bool sceneLoaded = sceneManager.loadScene("assets/scenes/room_01.json");

    resourceManager.loadItems("src/engine/assets/items.json"); 
    inventorySystem.initialize(sceneManager.getCurrentScene(), eventBus, resourceManager);

    combatSystem.initialize(sceneManager.getCurrentScene(), eventBus);
    projectileSystem.initialize(sceneManager.getCurrentScene(), eventBus);

    return sceneLoaded;
}

void Engine::run()
{
    using clock = std::chrono::steady_clock;

    auto lastTime = clock::now(); // get time in whatever format the clock uses

    while (running && !WindowShouldClose())
    {
        auto currentTime = clock::now();
        std::chrono::duration<float> elapsed = currentTime - lastTime; // convert result to seconds as float inside duration wrapper
        lastTime = currentTime;

        float dt = elapsed.count(); // get the float value from the duration wrapper

        update(dt);
        render();
    }
}

void Engine::update(float dt)
{
    bool gameplayPaused = dialogueManager.isActive();
    //std::cout << "Updating engine. dt: " << dt << '\n';
    inputSystem.update(sceneManager.getCurrentScene(), eventBus);
    //debugNPCInteraction();
    
    if (IsKeyPressed(KEY_K))
    {
        questManager.onEvent("enemy_killed", "enemy", 1);
    }
    
    if (!gameplayPaused)
    {
        interactionSystem.update(sceneManager.getCurrentScene(), eventBus);

        aiSystem.update(sceneManager.getCurrentScene(), eventBus, dt);

        movementSystem.update(sceneManager.getCurrentScene(), dt);
        projectileSystem.update(sceneManager.getCurrentScene(), dt);

        conditionalSystem.update(
            sceneManager.getCurrentScene(),
            conditionManager
        );

        collisionSystem.update(sceneManager.getCurrentScene(), eventBus);

        triggerSystem.update(
            sceneManager.getCurrentScene(),
            eventBus
        );
    }
    dialogueManager.update();
    eventBus.dispatch();
    cameraSystem.update(sceneManager.getCurrentScene(), camera);

    if (IsKeyPressed(KEY_NINE)) 
    {
        sceneManager.getCurrentScene().saveState(); //saves all entities with componenents to saved_entities.json for test
    }
    if (IsKeyPressed(KEY_Q))
    {
        conditionManager.debugUnlocked =
            !conditionManager.debugUnlocked;
    }

    sceneManager.update(dt);
    sceneManager.applyPendingSceneChange();

    sceneManager.getCurrentScene().cleanupDestroyedEntities();
}

void Engine::render()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
    renderSystem.render(sceneManager.getCurrentScene(), resourceManager, camera);
    inventoryUISystem.render(sceneManager.getCurrentScene(), resourceManager);
    dialogueManager.render();
    EndDrawing();
}

void Engine::shutdown()
{
    sceneManager.shutdown();
    CloseWindow();
    std::cout << "Engine shutdown\n";
}

void Engine::debugNPCInteraction()
{
    if (dialogueManager.isActive())
        return;

    Scene& scene = sceneManager.getCurrentScene();
    ComponentStorage& storage = scene.getComponentStorage();

    auto& players = storage.GetComponents<PlayerInput>();
    auto& npcs = storage.GetComponents<DialogueSource>();

    if (players.empty())
        return;

    if (!IsKeyPressed(KEY_E))
        return;

    Entity player = players.begin()->first;

    if (!storage.HasComponent<TransformComponent>(player))
        return;

    TransformComponent& playerTransform =
        storage.GetComponent<TransformComponent>(player);

    for (auto& [npc, dialogue] : npcs)
    {
        if (!storage.HasComponent<TransformComponent>(npc))
            continue;

        TransformComponent& npcTransform =
            storage.GetComponent<TransformComponent>(npc);

        float dx = playerTransform.x - npcTransform.x;
        float dz = playerTransform.z - npcTransform.z;

        float distSq = dx * dx + dz * dz;

        if (distSq < 2.25f)
        {
            dialogueManager.startDialogue(
                dialogue.dialogueSetId
            );

            break;
        }
    }
}

void Engine::onActionEvent(const ActionEvent& actionEvent){

    //IGNORE PICKUPS
    if (actionEvent.actionType == "PickupItem") return;

    //Find the action
    auto it = actionHandlers.find(actionEvent.actionType);

    //If the action excist
    if (it != actionHandlers.end()){
        it -> second(actionEvent.targetData);
    }else{
       std::cout << "VARNING: Unknown actionType from JSON: " << actionEvent.actionType << '\n';
    }
}