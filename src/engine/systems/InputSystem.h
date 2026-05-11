#pragma once
#include "../messaging/EventBus.h"
#include "../messaging/Event.h"

class Scene;

class InputSystem {
public:
    void update(Scene& scene, EventBus& eventBus);
};