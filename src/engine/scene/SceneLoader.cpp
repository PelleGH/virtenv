#include "SceneLoader.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool SceneLoader::loadFromFile(const std::string& path, SceneData& outData)
{
    outData.entities.clear();
    outData.playerSpawns.clear();
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
    if (sceneJson.contains("camera"))
    {
        const auto& cameraJson = sceneJson["camera"];

        if (cameraJson.contains("mode"))
            outData.camera.mode = cameraJson["mode"].get<std::string>();

        if (cameraJson.contains("position"))
        {
            outData.camera.positionX = cameraJson["position"][0].get<float>();
            outData.camera.positionY = cameraJson["position"][1].get<float>();
            outData.camera.positionZ = cameraJson["position"][2].get<float>();
        }

        if (cameraJson.contains("target"))
        {
            outData.camera.targetX = cameraJson["target"][0].get<float>();
            outData.camera.targetY = cameraJson["target"][1].get<float>();
            outData.camera.targetZ = cameraJson["target"][2].get<float>();
        }
    }
    if (sceneJson.contains("playerSpawns"))
    {
        for (const auto& spawnJson : sceneJson["playerSpawns"])
        {
            std::string id = spawnJson["id"];

            PlayerSpawn spawn;
            spawn.x = spawnJson["position"][0];
            spawn.y = spawnJson["position"][1];
            spawn.z = spawnJson["position"][2];

            if (spawnJson.contains("skinChoice")){
                spawn.skinChoice = spawnJson["skinChoice"];
            }

            outData.playerSpawns[id] = spawn;
        }
    }
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
        if (cubeJson.find("targetSpawn") != cubeJson.end())
            cube.targetSpawn = cubeJson["targetSpawn"];

        if (cubeJson.contains("modelID"))
        {
            cube.modelID = cubeJson["modelID"];
        }else{
            cube.modelID = "wall_model";
        }
        
        outData.cubes.push_back(cube);
    }
    outData.entities.clear();

    if (sceneJson.contains("entities"))
    {
        for (const auto& entityJson : sceneJson["entities"])
        {
            outData.entities.push_back(entityJson);
        }
    }

    return true;
}