#pragma once
#include "raylib.h"

class Scene;
class ResourceManager;

class RenderSystem {
public:
    void render(Scene& scene, ResourceManager& resourceManager, const Camera3D& camera);

private:
    void renderGrid(Scene& scene, ResourceManager& resourceManager);
    void renderEntities(Scene& scene, ResourceManager& resourceManager);
    void renderHealthBars(Scene& scene, const Camera3D& camera);
    void renderPlayerHealthBar(Scene& scene);
};