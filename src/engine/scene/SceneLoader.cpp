#include "SceneLoader.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool SceneLoader::loadFromFile(const std::string& path, SceneData& outData)
{
    std::ifstream file(path);

    if (!file.is_open())
    {
        std::cout << "Failed to open scene file: " << path << '\n';
        return false;
    }

    json sceneJson;
    file >> sceneJson;

    outData.name = sceneJson["name"];
    outData.cubes.clear();

    for (const auto& cubeJson : sceneJson["cubes"]) // instantiate cubes from json
    {
        GridCube cube;

        cube.position.x = cubeJson["position"][0];
        cube.position.y = cubeJson["position"][1];
        cube.position.z = cubeJson["position"][2];

        cube.type = cubeJson["type"];
        cube.solid = cubeJson["solid"];
        cube.trigger = cubeJson["trigger"];

        if (cubeJson.contains("targetScene")) // "door" cube that leads to another scene (just a flag for now)
            cube.targetScene = cubeJson["targetScene"];

        outData.cubes.push_back(cube);
    }

    return true;
}