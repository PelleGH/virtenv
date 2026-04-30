#pragma once
#include <string>
#include "raylib.h"
#include "SceneData.h"

class Scene
{
public:
    bool load(const std::string& scenePath);
    void update(float dt);
    void render(const Camera3D& camera);
    void unload();
private:
    std::string name;

    SceneData data;
};