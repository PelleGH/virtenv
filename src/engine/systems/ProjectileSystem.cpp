#include "ProjectileSystem.h"
#include "../scene/Scene.h"
#include "../ecs/Components.h"
#include "../messaging/EventBus.h"

void ProjectileSystem::initialize(Scene& scene, EventBus& eventBus) {
    m_scene = &scene;
    m_eventBus = &eventBus;

    // Subscribe to the OverlapEvent!
    eventBus.subscribe<OverlapEvent>([this](const OverlapEvent& e) {
        this->onOverlap(e);
    });
}

void ProjectileSystem::update(Scene& scene, float dt) {
    ComponentStorage& components = scene.getComponentStorage();
    
    // ONLY responsibility here is aging the projectiles out of existence
    for (auto& [entity, proj] : components.GetComponents<Projectile>()) {
        proj.timeToLive -= dt;
        if (proj.timeToLive <= 0.0f) {
            scene.queueEntityDestruction(entity);
        }
    }
}

void ProjectileSystem::onOverlap(const OverlapEvent& event) {
    if (!m_scene) return;
    ComponentStorage& components = m_scene->getComponentStorage();

    if (event.hitWall) {
        if (components.HasComponent<Projectile>(event.entityA)) {
            m_scene->queueEntityDestruction(event.entityA);
        }
        return; // We hit a wall, stop processing!
    }

    // Figure out if A is the projectile or B is the projectile
    Entity projectileEntity;
    Entity targetEntity;

    if (components.HasComponent<Projectile>(event.entityA)) {
        projectileEntity = event.entityA;
        targetEntity = event.entityB;
    } 
    else if (components.HasComponent<Projectile>(event.entityB)) {
        projectileEntity = event.entityB;
        targetEntity = event.entityA;
    } 
    else {
        return; // Neither entity is a projectile, ignore!
    }

    auto& proj = components.GetComponent<Projectile>(projectileEntity);

    // Prevent the projectile from instantly hitting the guy who shot it
    if (targetEntity == proj.owner) return;

    // If the thing we hit can take damage, attack it!
    if (components.HasComponent<Health>(targetEntity)) {
        AttackEvent attack;
        attack.attacker = proj.owner;
        attack.damage = proj.damage;
        attack.isDirectHit = true;
        attack.directTarget = targetEntity;
        
        m_eventBus->publish(attack);
    }

    // Regardless of what we hit (a player or a wall), destroy the projectile
    m_scene->queueEntityDestruction(projectileEntity);
}