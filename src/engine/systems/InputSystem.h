#pragma once
#include "../messaging/EventBus.h"

class Scene;

class InputSystem {
public:
    void update(Scene& scene, EventBus& eventBus);
};