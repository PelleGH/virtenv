#pragma once
#include "../messaging/Event.h"

class Scene;
class EventBus;

class ProjectileSystem {
public:
    void initialize(Scene& scene, EventBus& eventBus);
    void update(Scene& scene, float dt);

private:
    Scene* m_scene = nullptr;
    EventBus* m_eventBus = nullptr;

    void onOverlap(const OverlapEvent& event);
};