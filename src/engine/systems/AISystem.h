#pragma once

#include "../scene/Scene.h"
#include "../messaging/EventBus.h"
#include "../messaging/Event.h"
class AISystem
{
public:
    // Takes the scene to read components and the EventBus to broadcast attacks
    void update(Scene& scene, EventBus& eventBus);
};