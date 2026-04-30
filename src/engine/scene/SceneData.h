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

struct SceneData
{
    std::string name;
    std::vector<GridCube> cubes;

    GridPosition playerSpawn;
};

