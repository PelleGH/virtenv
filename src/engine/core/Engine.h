#pragma once
#include "raylib.h"
#include "engine/scene/SceneManager.h"
class Engine
{
public:
    bool init();
    void run();
    void shutdown();

private:
    bool running = false;

    SceneManager sceneManager;
    Camera3D camera;
    void update(float dt);
    void render();
};