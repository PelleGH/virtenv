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

    void publish(const AttackEvent& event)
    {
        attackEvents.push_back(event);
    }

    void subscribe(std::function<void(const AttackEvent&)> handler)
    {
        attackHandlers.push_back(handler);
    }

    void publish(const DeathEvent& event)
    {
        deathEvents.push_back(event);
    }

    void subscribe(std::function<void(const DeathEvent&)> handler)
    {
        deathHandlers.push_back(handler);
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

        for (const auto& event : attackEvents)
        {
            for (auto& handler : attackHandlers)
            {
                handler(event);
            }
        }
        attackEvents.clear();

        for (const auto& event : deathEvents)
        {
            for (auto& handler : deathHandlers)
            {
                handler(event);
            }
        }
        deathEvents.clear();
    }

private:
    std::vector<SceneTransitionEvent> sceneEvents;
    std::vector<std::function<void(const SceneTransitionEvent&)>> sceneHandlers;

    std::vector<AttackEvent> attackEvents;
    std::vector<std::function<void(const AttackEvent&)>> attackHandlers;

    std::vector<DeathEvent> deathEvents;
    std::vector<std::function<void(const DeathEvent&)>> deathHandlers;
};