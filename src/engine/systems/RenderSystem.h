#pragma once
#include "raylib.h"

class Scene;

class RenderSystem {
public:
    void render(Scene& scene, const Camera3D& camera);

private:
    void renderGrid(Scene& scene);
    void renderEntities(Scene& scene);
};