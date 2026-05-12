#include "ProjectileSystem.h"
#include "../scene/Scene.h"
#include "../ecs/Components.h"
#include "../messaging/EventBus.h"
#include "../messaging/Event.h"
#include <cmath>
#include <iostream>

void ProjectileSystem::update(Scene& scene, EventBus& eventBus, float dt) {
    ComponentStorage& components = scene.getComponentStorage();
    auto& projectiles = components.GetComponents<Projectile>();

    for (auto& [entity, proj] : projectiles) {
        if (!components.HasComponent<TransformComponent>(entity)) continue;

        auto& transform = components.GetComponent<TransformComponent>(entity);

        //Move the projectile
        transform.x += proj.velocityX * dt;
        transform.z += proj.velocityZ * dt;

        //Reduce Time to Live
        proj.timeToLive -= dt;
        if (proj.timeToLive <= 0.0f) {
            scene.queueEntityDestruction(entity);
            continue;
        }

        //Player Collision Check
        for (auto& [playerEntity, input] : components.GetComponents<PlayerInput>()) {
            if (!components.HasComponent<TransformComponent>(playerEntity)) continue;
            auto& playerTrans = components.GetComponent<TransformComponent>(playerEntity);

            float dx = transform.x - playerTrans.x;
            float dz = transform.z - playerTrans.z;
            float distSq = (dx * dx) + (dz * dz);

            //If it gets within 0.5 units of the player, it hits!
            if (distSq < 0.5f) {
                //Apply Damage
                if (components.HasComponent<Health>(playerEntity)) {
                    auto& hp = components.GetComponent<Health>(playerEntity);
                    hp.current -= proj.damage;
                    std::cout << "Player hit by projectile! HP: " << hp.current << '\n';

                    if (hp.current <= 0) {
                        eventBus.publish(DeathEvent{playerEntity});
                    }
                }
                
                // Destroy projectile on impact
                scene.queueEntityDestruction(entity);
                break; 
            }
        }
    }
}