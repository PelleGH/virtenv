#include "Engine.h"

#include <chrono>
#include <iostream>

bool Engine::init()
{
    InitWindow(1280, 720, "Virtenv");
    SetTargetFPS(60);

    camera = { 0 };
    camera.position = { 5.0f, 5.0f, 5.0f };
    camera.target = { 0.0f, 0.0f, 0.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

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
    std::cout << "Updating engine. dt: " << dt << '\n';
    inputSystem.update(sceneManager.getCurrentScene());
    movementSystem.update(sceneManager.getCurrentScene(), dt);
    if (IsKeyPressed(KEY_ONE)) // TODO: temporary way to switch scenes for testing, will be moved to eventbus when it's implemented
        sceneManager.requestSceneChange("assets/scenes/room_01.json");

    if (IsKeyPressed(KEY_TWO))
        sceneManager.requestSceneChange("assets/scenes/room_02.json");

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