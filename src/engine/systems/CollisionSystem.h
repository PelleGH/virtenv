#pragma once

class Scene;

class CollisionSystem {
public:
    void update(Scene& scene);

private:
    bool collidesWithSolidGrid(Scene& scene, float x, float y, float z, float width, float height, float depth);
};