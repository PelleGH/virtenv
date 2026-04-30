#pragma once
#include "raylib.h"
#include "engine/scene/Scene.h"
class Engine
{
public:
    bool init();
    void run();
    void shutdown();

private:
    bool running = false;

    Scene currentScene;
    Camera3D camera;
    void update(float dt);
    void render();
};