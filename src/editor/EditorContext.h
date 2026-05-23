#pragma once

#include <string>
#include <vector>

#include "engine/scene/SceneData.h"
#include "engine/scene/Scene.h"
#include "raylib.h"
#include "engine/systems/RenderSystem.h"
#include "engine/resource/ResourceManager.h"

enum class SelectionType
{
    None,
    GridCube,
    Entity,
    SpawnPoint
};

struct EditorSelection
{
    SelectionType type = SelectionType::None;
    int index = -1;
};

struct EditorContext
{
    bool viewportHovered = false;
    bool previewDirty = false;
    bool buildOutdated = false;
    
    nlohmann::json componentSchemas;
    
    SceneData scene;
    Scene previewScene;
    ResourceManager resourceManager;
    RenderSystem renderSystem;


    std::string projectPath = ""; 
    std::string projectName = "No Project Loaded";
    std::vector<std::string> availableProjects;
 
    std::string currentScenePath = "assets/scenes/room_01.json";
    std::vector<std::string> scenePaths;
    int currentSceneIndex = 0;
    
    EditorSelection selection;

    Camera3D editorCamera{};
    RenderTexture2D viewportTexture{};
    bool viewportReady = false;

    bool sceneLoaded = false;
    bool dirty = false;
};