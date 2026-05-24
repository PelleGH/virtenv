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
enum class GridPainterMode
{
    Paint,
    Erase,
    Eyedropper
};
struct GridPainterUndoAction
{
    bool hadCubeBefore = false;
    GridCube cubeBefore;

    bool hadCubeAfter = false;
    GridCube cubeAfter;
};
struct CubeTemplate
{
    std::string name = "Template";

    std::string type = "wall";
    bool solid = true;
    bool trigger = false;
    bool visible = true;

    std::string targetScene = "";
    std::string targetSpawn = "";
    std::string modelID = "";
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
    std::string startingScene;
    std::string currentScenePath = "assets/scenes/room_01.json";
    std::vector<std::string> scenePaths;
    int currentSceneIndex = 0;
    
    EditorSelection selection;

    Camera3D editorCamera{};
    RenderTexture2D viewportTexture{};
    bool viewportReady = false;

    bool previewSceneCamera = false;

    bool sceneLoaded = false;
    bool dirty = false;

    //for scene switching (saving)
    bool showUnsavedScenePopup = false;
    int pendingSceneIndex = -1;

    //for editor saving
    bool showUnsavedExitPopup = false;
    bool exitRequested = false;

    bool gridPainterEnabled = false;
    int gridPainterLayerY = 0;
    int selectedCubeTemplate = 0;
    GridPainterMode gridPainterMode = GridPainterMode::Paint;

    CubeTemplate cubeTemplates[5] = {
        { "Floor", "floor", false, false, true, "", "", "" },
        { "Wall", "wall", true, false, true, "", "", "wall_model" },
        { "Door", "door", false, true, true, "", "", "wall_model" },
        { "Trigger", "trigger", false, true, true, "", "", "" },
        { "Custom", "wall", true, false, true, "", "", "wall_model" }
    };
    std::vector<GridPainterUndoAction> gridPainterUndoStack;
};

