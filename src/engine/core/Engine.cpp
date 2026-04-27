#include "Engine.h"

#include <chrono>
#include <iostream>

bool Engine::init()
{
    std::cout << "Engine initialized\n";
    running = true;
    return true;
}

void Engine::run()
{
    using clock = std::chrono::high_resolution_clock;

    auto lastTime = clock::now(); // get time in whatever format the clock uses

    while (running)
    {
        auto currentTime = clock::now();
        std::chrono::duration<float> elapsed = currentTime - lastTime; // convert result to seconds as float inside duration wrapper
        lastTime = currentTime;

        float dt = elapsed.count(); // get the float value from the duration wrapper

        update(dt);
        render();

        // temporary so we don't have an infinite loop while working
        static int frameCount = 0;
        frameCount++;

        if (frameCount > 300)
            running = false;
    }
}

void Engine::update(float dt)
{
    std::cout << "Updating engine. dt: " << dt << '\n';
}

void Engine::render()
{

}

void Engine::shutdown()
{
    std::cout << "Engine shutdown\n";
}