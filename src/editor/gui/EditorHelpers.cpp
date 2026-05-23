#include "EditorPanels.h"
#include <fstream>
#include <filesystem>
#include <vector>
#include <imgui.h>
#include <engine/scene/SceneLoader.h>
#include <editor/project/ProjectManager.h>

void MarkSceneChanged(EditorContext& context)
{
    context.dirty = true;
    context.previewDirty = true;
    context.buildOutdated = true;
}

void AddCube(EditorContext& context, const std::string& type)
{
    GridCube cube;
    cube.type = type;
    cube.position = {0, 0, 0};
    cube.trigger = false;
    cube.visible = true;
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
void AddEntity(EditorContext& context, const std::string& type)
{
    nlohmann::json entity;

    entity["name"] = type;

    entity["TransformComponent"] = {
        {"x", 0.0f},
        {"y", 1.0f},
        {"z", 0.0f}
    };

    entity["Renderer"] = {
        {"modelID", ""},
        {"textureID", ""},
        {"scale", 0.5f},
        {"width", 0.5f},
        {"height", 0.5f},
        {"depth", 0.5f},
        {"color", {255, 255, 255, 255}},
        {"zIndex", 0}
    };

    entity["Collider"] = {
        {"width", 0.5f},
        {"height", 0.5f},
        {"depth", 0.5f},
        {"offsetX", 0.0f},
        {"offsetY", 0.0f},
        {"offsetZ", 0.0f},
        {"isTrigger", false},
        {"enabled", true}
    };

    if (type == "NPC")
    {
        entity["DialogueSource"] = {
            {"dialogueSetId", ""}
        };
    }
    else if (type == "Enemy")
    {
        entity["Health"] = {
            {"current", 30},
            {"max", 30},
            {"defense", 0}
        };

        entity["Attack"] = {
            {"damage", 5},
            {"baseDamage", 5},
            {"cooldown", 2.0f},
            {"range", 2.0f},
            {"isRanged", false},
            {"projectileSpeed", 5.0f},
            {"enemyType", "enemy"}
        };
    }
    else if (type == "Pickup")
    {
        entity["Interactable"] = {
            {"interactionRadius", 1.0f},
            {"actions", nlohmann::json::array()}
        };
    }

    context.scene.entities.push_back(entity);

    context.selection.type = SelectionType::Entity;
    context.selection.index = (int)context.scene.entities.size() - 1;

    MarkSceneChanged(context);
}
void LoadComponentSchemas(EditorContext& context)
{
    std::vector<std::filesystem::path> possiblePaths = {
        "src/editor/config/component_schemas.json",
        "../src/editor/config/component_schemas.json",
        "../../src/editor/config/component_schemas.json",
        "../../../src/editor/config/component_schemas.json"
    };

    context.componentSchemas = nlohmann::json::object();

    for (const auto& schemaPath : possiblePaths)
    {
        std::ifstream file(schemaPath);

        if (!file.is_open())
            continue;

        try
        {
            file >> context.componentSchemas;
            return;
        }
        catch (...)
        {
            context.componentSchemas = nlohmann::json::object();
            return;
        }
    }
}
void LoadEditorScene(EditorContext& context, int index)
{
    LoadComponentSchemas(context);

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

GridCube MakeCubeFromTemplate(
    const CubeTemplate& cubeTemplate,
    int x,
    int y,
    int z
)
{
    GridCube cube;

    cube.position = { x, y, z };

    cube.type = cubeTemplate.type;
    cube.solid = cubeTemplate.solid;
    cube.trigger = cubeTemplate.trigger;
    cube.visible = cubeTemplate.visible;
    cube.targetScene = cubeTemplate.targetScene;
    cube.targetSpawn = cubeTemplate.targetSpawn;
    cube.modelID = cubeTemplate.modelID;

    return cube;
}

int FindCubeAt(EditorContext& context, int x, int y, int z)
{
    for (int i = 0; i < (int)context.scene.cubes.size(); i++)
    {
        const GridCube& cube = context.scene.cubes[i];

        if (cube.position.x == x &&
            cube.position.y == y &&
            cube.position.z == z)
        {
            return i;
        }
    }

    return -1;
}

void PaintCubeAt(EditorContext& context, int x, int y, int z)
{
    if (context.selectedCubeTemplate < 0 || context.selectedCubeTemplate >= 5)
        return;

    GridCube newCube = MakeCubeFromTemplate(
        context.cubeTemplates[context.selectedCubeTemplate],
        x,
        y,
        z
    );

    int existingIndex = FindCubeAt(context, x, y, z);

    // Avoid filling undo stack while dragging over the same cell
    if (existingIndex >= 0)
    {
        const GridCube& existingCube = context.scene.cubes[existingIndex];

        if (existingCube.type == newCube.type &&
            existingCube.solid == newCube.solid &&
            existingCube.trigger == newCube.trigger &&
            existingCube.visible == newCube.visible &&
            existingCube.targetScene == newCube.targetScene &&
            existingCube.targetSpawn == newCube.targetSpawn &&
            existingCube.modelID == newCube.modelID)
        {
            return;
        }
    }

    GridPainterUndoAction undoAction;

    if (existingIndex >= 0)
    {
        undoAction.hadCubeBefore = true;
        undoAction.cubeBefore = context.scene.cubes[existingIndex];

        context.scene.cubes[existingIndex] = newCube;
        context.selection = { SelectionType::GridCube, existingIndex };
    }
    else
    {
        undoAction.hadCubeBefore = false;

        context.scene.cubes.push_back(newCube);
        context.selection = {
            SelectionType::GridCube,
            (int)context.scene.cubes.size() - 1
        };
    }

    undoAction.hadCubeAfter = true;
    undoAction.cubeAfter = newCube;

    context.gridPainterUndoStack.push_back(undoAction);

    MarkSceneChanged(context);
}

void EraseCubeAt(EditorContext& context, int x, int y, int z)
{
    int existingIndex = FindCubeAt(context, x, y, z);

    if (existingIndex < 0)
        return;

    GridPainterUndoAction undoAction;
    undoAction.hadCubeBefore = true;
    undoAction.cubeBefore = context.scene.cubes[existingIndex];
    undoAction.hadCubeAfter = false;

    context.scene.cubes.erase(context.scene.cubes.begin() + existingIndex);

    context.selection = { SelectionType::None, -1 };

    context.gridPainterUndoStack.push_back(undoAction);

    MarkSceneChanged(context);
}

void CopyCubeToTemplate(EditorContext& context, const GridCube& cube)
{
    if (context.selectedCubeTemplate < 0 || context.selectedCubeTemplate >= 5)
        return;

    CubeTemplate& cubeTemplate = context.cubeTemplates[context.selectedCubeTemplate];

    cubeTemplate.name = cube.type + " template";
    cubeTemplate.type = cube.type;
    cubeTemplate.solid = cube.solid;
    cubeTemplate.trigger = cube.trigger;
    cubeTemplate.targetScene = cube.targetScene;
    cubeTemplate.targetSpawn = cube.targetSpawn;
    cubeTemplate.modelID = cube.modelID;
}
void UndoGridPainterAction(EditorContext& context)
{
    if (context.gridPainterUndoStack.empty())
        return;

    GridPainterUndoAction undoAction = context.gridPainterUndoStack.back();
    context.gridPainterUndoStack.pop_back();

    int x = 0;
    int y = 0;
    int z = 0;

    if (undoAction.hadCubeAfter)
    {
        x = undoAction.cubeAfter.position.x;
        y = undoAction.cubeAfter.position.y;
        z = undoAction.cubeAfter.position.z;
    }
    else if (undoAction.hadCubeBefore)
    {
        x = undoAction.cubeBefore.position.x;
        y = undoAction.cubeBefore.position.y;
        z = undoAction.cubeBefore.position.z;
    }
    else
    {
        return;
    }

    int currentIndex = FindCubeAt(context, x, y, z);

    if (undoAction.hadCubeBefore)
    {
        if (currentIndex >= 0)
        {
            context.scene.cubes[currentIndex] = undoAction.cubeBefore;
            context.selection = { SelectionType::GridCube, currentIndex };
        }
        else
        {
            context.scene.cubes.push_back(undoAction.cubeBefore);
            context.selection = {
                SelectionType::GridCube,
                (int)context.scene.cubes.size() - 1
            };
        }
    }
    else
    {
        if (currentIndex >= 0)
        {
            context.scene.cubes.erase(context.scene.cubes.begin() + currentIndex);
        }

        context.selection = { SelectionType::None, -1 };
    }

    MarkSceneChanged(context);
}

void SaveCurrentEditorScene(EditorContext& context)
{
    if (context.currentScenePath.empty())
        return;

    if (!SceneLoader::saveToFile(context.currentScenePath, context.scene))
    {
        std::cout << "[Editor] ERROR: Failed to save scene: "
                  << context.currentScenePath << std::endl;
        return;
    }

    context.dirty = false;
    context.previewDirty = false;

    std::cout << "[Editor] Saved scene: "
              << context.currentScenePath << std::endl;
}

void RequestLoadEditorScene(EditorContext& context, int index)
{
    if (index < 0 || index >= (int)context.scenePaths.size())
        return;

    if (index == context.currentSceneIndex)
        return;

    if (context.dirty)
    {
        context.pendingSceneIndex = index;
        context.showUnsavedScenePopup = true;
        return;
    }

    LoadEditorScene(context, index);
}

void DrawUnsavedScenePopup(EditorContext& context)
{
    if (context.showUnsavedScenePopup)
    {
        ImGui::OpenPopup("Unsaved Scene Changes");
        context.showUnsavedScenePopup = false;
    }

    if (ImGui::BeginPopupModal(
            "Unsaved Scene Changes",
            nullptr,
            ImGuiWindowFlags_AlwaysAutoResize))
    {
        std::string sceneName = std::filesystem::path(context.currentScenePath).stem().string();

        ImGui::Text("Unsaved changes in scene \"%s\".", sceneName.c_str());
        ImGui::Text("Would you like to save before switching scenes?");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Yes, Save", ImVec2(120, 0)))
        {
            SaveCurrentEditorScene(context);

            if (context.pendingSceneIndex >= 0)
            {
                LoadEditorScene(context, context.pendingSceneIndex);
            }

            context.pendingSceneIndex = -1;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("No, Discard", ImVec2(120, 0)))
        {
            if (context.pendingSceneIndex >= 0)
            {
                LoadEditorScene(context, context.pendingSceneIndex);
            }

            context.pendingSceneIndex = -1;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            context.pendingSceneIndex = -1;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

bool RequestEditorExit(EditorContext& context)
{
    if (context.dirty)
    {
        context.showUnsavedExitPopup = true;
        return false;
    }

    context.exitRequested = true;
    return true;
}

void DrawUnsavedExitPopup(EditorContext& context)
{
    if (context.showUnsavedExitPopup)
    {
        ImGui::OpenPopup("Unsaved Project Changes");
        context.showUnsavedExitPopup = false;
    }

    if (ImGui::BeginPopupModal(
            "Unsaved Project Changes",
            nullptr,
            ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("You have unsaved changes.");
        ImGui::Text("Would you like to save before closing the editor?");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Yes, Save", ImVec2(120, 0)))
        {
            SaveProject(context);
            context.exitRequested = true;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("No, Discard", ImVec2(120, 0)))
        {
            context.exitRequested = true;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            context.exitRequested = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}