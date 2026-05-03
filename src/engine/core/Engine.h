#pragma once
#include "raylib.h"
#include "engine/scene/SceneManager.h"
#include "../systems/RenderSystem.h"
#include "../systems/InputSystem.h"
#include "../systems/MovementSystem.h"
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
    Camera3D camera;
    void update(float dt);
    void render();
};