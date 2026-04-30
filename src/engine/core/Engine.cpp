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
    return currentScene.load("assets/scenes/room_01.json");
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

        // // temporary so we don't have an infinite loop while working
        // static int frameCount = 0;
        // frameCount++;

        // if (frameCount > 300)
        //     running = false;
    }
}

void Engine::update(float dt)
{
    std::cout << "Updating engine. dt: " << dt << '\n';
    currentScene.update(dt);
}

void Engine::render()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
    currentScene.render(camera);
    EndDrawing();
}

void Engine::shutdown()
{
    currentScene.unload();
    CloseWindow();
    std::cout << "Engine shutdown\n";
}