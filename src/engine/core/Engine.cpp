#include "Engine.h"
#include "engine/scene/Scene.h"
#include <chrono>
#include <iostream>

bool Engine::init()
{
    InitWindow(1280, 720, "Virtenv");
    SetTargetFPS(60);

    resourceManager.LoadFromManifest("src/engine/assets/assets.json");
    sceneManager.init(resourceManager);

    cameraSystem.init(camera);

    questManager.setConditionManager(&conditionManager);
    questManager.loadQuests("assets/quests.json");
    dialogueManager.setQuestManager(&questManager);

    eventBus.subscribe([this](const SceneTransitionEvent& event)
    {
        sceneManager.requestSceneChange(
            "assets/scenes/" + event.targetScene + ".json",
            event.targetSpawn
        );
    });
    running = true;

    return sceneManager.loadScene("assets/scenes/room_01.json");
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
    inputSystem.update(sceneManager.getCurrentScene());
    debugNPCInteraction();
    if (IsKeyPressed(KEY_K))
    {
        questManager.onEvent("enemy_killed", "enemy");
    }
    if (!gameplayPaused){
        movementSystem.update(sceneManager.getCurrentScene(), dt);
        conditionalSystem.update(sceneManager.getCurrentScene(), conditionManager);
        collisionSystem.update(sceneManager.getCurrentScene());
        triggerSystem.update(sceneManager.getCurrentScene(), eventBus);
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
}

void Engine::render()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
    renderSystem.render(sceneManager.getCurrentScene(), resourceManager, camera);
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