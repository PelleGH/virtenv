#pragma once
#include "../messaging/EventBus.h"
#include "../messaging/Event.h"
class Scene;
class TriggerSystem
{
public:
    void update(Scene& scene, EventBus& eventBus);

private:
    bool overlapsTrigger(
        float entityX,
        float entityY,
        float entityZ,
        float width,
        float height,
        float depth,
        float triggerX,
        float triggerY,
        float triggerZ
    );
};