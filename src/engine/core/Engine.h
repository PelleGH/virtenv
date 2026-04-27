#pragma once

class Engine
{
public:
    bool init();
    void run();
    void shutdown();

private:
    bool running = false;

    void update(float dt);
    void render();
};