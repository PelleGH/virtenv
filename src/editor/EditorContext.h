#pragma once

#include <string>
#include "engine/scene/SceneData.h"
#include "raylib.h"

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
    std::string currentScenePath = "assets/scenes/room_01.json";

    EditorSelection selection;

    Camera3D editorCamera{};
    RenderTexture2D viewportTexture{};
    bool viewportReady = false;

    bool sceneLoaded = false;
    bool dirty = false;
};