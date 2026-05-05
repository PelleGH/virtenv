#pragma once

#include <string>
#include <vector>

struct GridPosition
{
    int x = 0;
    int y = 0;
    int z = 0;
};

struct GridCube
{
    GridPosition position;

    std::string type;       // "floor", "wall", "door"
    bool solid = false;
    bool trigger = false;

    std::string targetScene; // used later for doors
};
struct SceneCameraData
{
    std::string mode = "followPlayer"; // "followPlayer" or "fixed"

    float positionX = 5.0f;
    float positionY = 5.0f;
    float positionZ = 5.0f;

    float targetX = 0.0f;
    float targetY = 0.0f;
    float targetZ = 0.0f;
};
struct SceneData
{
    std::string name;
    std::vector<GridCube> cubes;

    GridPosition playerSpawn;
    SceneCameraData camera;
};

