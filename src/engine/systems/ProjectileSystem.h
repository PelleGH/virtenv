#pragma once
class Scene;
class EventBus;

class ProjectileSystem {
public:
    void update(Scene& scene, EventBus& eventBus, float dt);
};