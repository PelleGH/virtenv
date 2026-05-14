#pragma once
#include "engine/scene/Scene.h"
#include "DialogueManager.h"
#include "engine/messaging/EventBus.h"

class InteractionSystem{
    public: 
        void update(Scene& scene, EventBus& eventBus);
};