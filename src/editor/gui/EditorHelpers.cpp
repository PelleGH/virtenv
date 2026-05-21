#include "EditorPanels.h"

void MarkSceneChanged(EditorContext& context)
{
    context.dirty = true;
    context.previewDirty = true;
}

void AddCube(EditorContext& context, const std::string& type)
{
    GridCube cube;
    cube.type = type;
    cube.position = {0, 0, 0};
    cube.trigger = false;

    cube.solid = type == "wall";

    if (type == "trigger")
        cube.trigger = true;

    if (type == "door")
    {
        cube.solid = false;
        cube.trigger = true;
    }

    context.scene.cubes.push_back(cube);

    context.selection.type = SelectionType::GridCube;
    context.selection.index = (int)context.scene.cubes.size() - 1;

    MarkSceneChanged(context);
}

void AddSpawnPoint(EditorContext& context)
{
    PlayerSpawn spawn;
    spawn.x = 0.0f;
    spawn.y = 1.0f;
    spawn.z = 0.0f;
    spawn.skinChoice = 0;

    std::string id = "spawn_" + std::to_string(context.scene.playerSpawns.size());

    context.scene.playerSpawns[id] = spawn;

    context.selection.type = SelectionType::SpawnPoint;
    context.selection.index = (int)context.scene.playerSpawns.size() - 1;

    MarkSceneChanged(context);
}

void LoadEditorScene(EditorContext& context, int index)
{
    if (index < 0 || index >= (int)context.scenePaths.size())
        return;

    context.currentSceneIndex = index;
    context.currentScenePath = context.scenePaths[index];

    context.previewScene.load(context.currentScenePath);
    context.scene = context.previewScene.getData();

    context.selection = {};
    context.sceneLoaded = true;
    context.dirty = false;
    context.previewDirty = false;
}