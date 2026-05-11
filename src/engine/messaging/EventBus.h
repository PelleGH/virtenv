// Generic event bus used for communication between systems/managers.
//
// Systems publish events when something happens.
// Other systems can subscribe to specific event types.
//
// Example:
// eventBus.publish(EnemyKilledEvent{...});
//
// eventBus.subscribe<EnemyKilledEvent>([](const EnemyKilledEvent& e) {
//     // react to enemy death
// });
//
// Events are queued and processed later through dispatch().

#pragma once

#include <functional>
#include <memory>
#include <queue>
#include <typeindex>
#include <unordered_map>
#include <vector>

class EventBus
{
public:
    template<typename EventType>
    void publish(const EventType& event)
    {
        std::type_index type = std::type_index(typeid(EventType));

        if (eventQueues.find(type) == eventQueues.end())
        {
            eventQueues[type] = std::make_unique<EventQueue<EventType>>();
        }

        auto* queue =
            static_cast<EventQueue<EventType>*>(eventQueues[type].get());

        queue->events.push(event);
    }

    template<typename EventType>
    void subscribe(std::function<void(const EventType&)> handler)
    {
        std::type_index type = std::type_index(typeid(EventType));

        if (eventQueues.find(type) == eventQueues.end())
        {
            eventQueues[type] = std::make_unique<EventQueue<EventType>>();
        }

        auto* queue =
            static_cast<EventQueue<EventType>*>(eventQueues[type].get());

        queue->handlers.push_back(handler);
    }

    void dispatch()
    {
        for (auto& [type, queue] : eventQueues)
        {
            queue->dispatch();
        }
    }

    void clear()
    {
        eventQueues.clear();
    }

private:
    struct IEventQueue
    {
        virtual ~IEventQueue() = default;
        virtual void dispatch() = 0;
    };

    template<typename EventType>
    struct EventQueue : IEventQueue
    {
        std::queue<EventType> events;
        std::vector<std::function<void(const EventType&)>> handlers;

        void dispatch() override
        {
            while (!events.empty())
            {
                EventType event = events.front();
                events.pop();

                for (auto& handler : handlers)
                {
                    handler(event);
                }
            }
        }
    };

    std::unordered_map<std::type_index, std::unique_ptr<IEventQueue>> eventQueues;
};