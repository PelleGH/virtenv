#pragma once
#include "raylib.h"

class Scene;
class ResourceManager;
struct RenderOptions
{
    bool drawHealthBars = true;
    bool drawPlayerUI = true;
};
class RenderSystem {
public:
    void render(Scene& scene, ResourceManager& resourceManager, const Camera3D& camera);
    void render(Scene& scene, ResourceManager& resourceManager, const Camera3D& camera, const RenderOptions& options);

private:
    void renderGrid(Scene& scene, ResourceManager& resourceManager);
    void renderEntities(Scene& scene, ResourceManager& resourceManager);
    void renderHealthBars(Scene& scene, const Camera3D& camera);
    void renderPlayerHealthBar(Scene& scene);
};