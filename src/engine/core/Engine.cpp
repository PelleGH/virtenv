#include "Engine.h"

#include <chrono>
#include <iostream>

bool Engine::init()
{
    InitWindow(1280, 720, "Virtenv");
    SetTargetFPS(60);

    cameraSystem.init(camera);
    eventBus.subscribe([this](const SceneTransitionEvent& event)
    {
        sceneManager.requestSceneChange("assets/scenes/" + event.targetScene + ".json");
    });
    running = true;
    bool sceneLoaded = sceneManager.loadScene("assets/scenes/room_01.json");

    combatSystem.initialize(sceneManager.getCurrentScene(), eventBus);

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
    //std::cout << "Updating engine. dt: " << dt << '\n';
    inputSystem.update(sceneManager.getCurrentScene(), eventBus);
    aiSystem.update(sceneManager.getCurrentScene(), eventBus);
    movementSystem.update(sceneManager.getCurrentScene(), dt);
    collisionSystem.update(sceneManager.getCurrentScene());
    triggerSystem.update(sceneManager.getCurrentScene(), eventBus);
    eventBus.dispatch();
    cameraSystem.update(sceneManager.getCurrentScene(), camera);
    if (IsKeyPressed(KEY_ONE)) // TODO: temporary way to switch scenes for testing, will be moved to eventbus when it's implemented
        sceneManager.requestSceneChange("assets/scenes/room_01.json");

    if (IsKeyPressed(KEY_TWO))
        sceneManager.requestSceneChange("assets/scenes/room_02.json");

    if (IsKeyPressed(KEY_NINE)) 
    {
        sceneManager.getCurrentScene().saveState(); //saves all entities with componenents to saved_entities.json for test
    }

    sceneManager.update(dt);
    sceneManager.applyPendingSceneChange();
}

void Engine::render()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
    renderSystem.render(sceneManager.getCurrentScene(), camera);
    EndDrawing();
}

void Engine::shutdown()
{
    sceneManager.shutdown();
    CloseWindow();
    std::cout << "Engine shutdown\n";
}