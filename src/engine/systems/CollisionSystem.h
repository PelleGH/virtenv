#pragma once
#include "../ecs/Entity.h"
class Scene;

class CollisionSystem {
public:
    void update(Scene& scene);

private:
    bool collidesWithSolidGrid(Scene& scene, float x, float y, float z, float width, float height, float depth);
    bool collidesWithSolidEntities(
    Scene& scene,
    Entity self,
    float x,
    float y,
    float z,
    float width,
    float height,
    float depth
    );

    bool overlapsBox(
        float ax, float ay, float az,
        float aw, float ah, float ad,
        float bx, float by, float bz,
        float bw, float bh, float bd
    );
};