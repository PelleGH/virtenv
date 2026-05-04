#pragma once

#include <vector>
#include <functional>

#include "Event.h"

class EventBus
{
public:
    void publish(const SceneTransitionEvent& event) // might need a guard to prevent multiple publishes in a frame (maybe?)
    {
        sceneEvents.push_back(event);
    }

    void subscribe(std::function<void(const SceneTransitionEvent&)> handler)
    {
        sceneHandlers.push_back(handler);
    }

    void dispatch()
    {
        for (const auto& event : sceneEvents)
        {
            for (auto& handler : sceneHandlers)
            {
                handler(event);
            }
        }

        sceneEvents.clear();
    }

private:
    std::vector<SceneTransitionEvent> sceneEvents;
    std::vector<std::function<void(const SceneTransitionEvent&)>> sceneHandlers;
};