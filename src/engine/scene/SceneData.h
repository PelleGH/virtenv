#pragma once
#include <unordered_map>
#include <nlohmann/json.hpp>
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
    std::string targetSpawn; // used for triggers to specify where the player should spawn in the next scene
    std::string targetScene; // used later for doors
};
struct PlayerSpawn
{
    float x = 0.0f;
    float y = 1.0f;
    float z = 0.0f;
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
    std::vector<nlohmann::json> entities; 
    std::unordered_map<std::string, PlayerSpawn> playerSpawns;
    GridPosition playerSpawn;
    SceneCameraData camera;
};

