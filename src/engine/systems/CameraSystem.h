#pragma once

#include "raylib.h"

class Scene;

class CameraSystem
{
public:
    void init(Camera3D& camera);
    void update(Scene& scene, Camera3D& camera);
};