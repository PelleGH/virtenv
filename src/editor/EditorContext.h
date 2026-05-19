#pragma once

#include <string>
#include "engine/scene/SceneData.h"
#include <vector>

enum class SelectionType
{
    None,
    GridCube,
    Entity
};

struct EditorSelection
{
    SelectionType type = SelectionType::None;
    int index = -1;
};

struct EditorContext
{
    SceneData scene;

    std::string projectPath = ""; 
    std::string projectName = "No Project Loaded";
    std::vector<std::string> availableProjects;
 
    std::string currentScenePath = "assets/scenes/room_01.json";

    EditorSelection selection;

    bool sceneLoaded = false;
    bool dirty = false;
};