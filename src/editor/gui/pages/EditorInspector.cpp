#include "../EditorPanels.h"
#include "imgui.h"
#include "../../../engine/scene/SceneLoader.h"

#include <cstring>
#include <vector>
#include <string>

std::vector<std::string> GetSpawnIdsForScene(const std::string& scenePath)
{
    SceneData targetData;

    if (!SceneLoader::loadFromFile(scenePath, targetData))
        return {};

    std::vector<std::string> ids;

    for (const auto& [id, spawn] : targetData.playerSpawns)
        ids.push_back(id);

    return ids;
}

void DrawInspector(EditorContext& context)
{
    ImGui::Begin("Inspector");

    if (context.selection.type == SelectionType::None)
    {
        ImGui::Text("Nothing selected.");
        ImGui::End();
        return;
    }

    if (context.selection.type == SelectionType::GridCube)
    {
        int index = context.selection.index;

        if (index < 0 || index >= (int)context.scene.cubes.size())
        {
            ImGui::Text("Invalid cube selection.");
            ImGui::End();
            return;
        }

        auto& cube = context.scene.cubes[index];

        ImGui::Text("Grid Cube");

        if (ImGui::InputInt("X", &cube.position.x)) MarkSceneChanged(context);
        if (ImGui::InputInt("Y", &cube.position.y)) MarkSceneChanged(context);
        if (ImGui::InputInt("Z", &cube.position.z)) MarkSceneChanged(context);

        ImGui::SeparatorText("Appearance");

        char modelBuffer[128];
        strncpy(modelBuffer, cube.modelID.c_str(), sizeof(modelBuffer));
        modelBuffer[sizeof(modelBuffer) - 1] = '\0';

        if (ImGui::InputText("Model ID", modelBuffer, sizeof(modelBuffer)))
        {
            cube.modelID = modelBuffer;
            MarkSceneChanged(context);
        }

        ImGui::SeparatorText("Type");

        const char* types[] = { "floor", "wall", "door", "trigger" };

        int currentType = 0;
        for (int i = 0; i < 4; i++)
        {
            if (cube.type == types[i])
                currentType = i;
        }

        if (ImGui::Combo("Cube Type", &currentType, types, IM_ARRAYSIZE(types)))
        {
            cube.type = types[currentType];

            if (cube.type == "floor")
            {
                cube.solid = false;
                cube.trigger = false;
            }
            else if (cube.type == "wall")
            {
                cube.solid = true;
                cube.trigger = false;
            }
            else if (cube.type == "door" || cube.type == "trigger")
            {
                cube.solid = false;
                cube.trigger = true;
            }

            MarkSceneChanged(context);
        }

        if (ImGui::Checkbox("Solid", &cube.solid)) MarkSceneChanged(context);
        if (ImGui::Checkbox("Trigger", &cube.trigger)) MarkSceneChanged(context);
        if (cube.type == "door" || cube.trigger)
        {
            ImGui::SeparatorText("Scene Transition");

            const char* currentTargetScene =
                cube.targetScene.empty() ? "None" : cube.targetScene.c_str();

            if (ImGui::BeginCombo("Target Scene", currentTargetScene))
            {
                if (ImGui::Selectable("None", cube.targetScene.empty()))
                {
                    cube.targetScene.clear();
                    cube.targetSpawn.clear();
                    MarkSceneChanged(context);
                }

                for (const auto& scenePath : context.scenePaths)
                {
                    bool selected = cube.targetScene == scenePath;

                    if (ImGui::Selectable(scenePath.c_str(), selected))
                    {
                        cube.targetScene = scenePath;
                        cube.targetSpawn.clear();
                        MarkSceneChanged(context);
                    }

                    if (selected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }

            if (!cube.targetScene.empty())
            {
                std::vector<std::string> spawnIds = GetSpawnIdsForScene(cube.targetScene);

                const char* currentTargetSpawn =
                    cube.targetSpawn.empty() ? "None" : cube.targetSpawn.c_str();

                if (ImGui::BeginCombo("Target Spawn", currentTargetSpawn))
                {
                    if (ImGui::Selectable("None", cube.targetSpawn.empty()))
                    {
                        cube.targetSpawn.clear();
                        MarkSceneChanged(context);
                    }

                    for (const auto& spawnId : spawnIds)
                    {
                        bool selected = cube.targetSpawn == spawnId;

                        if (ImGui::Selectable(spawnId.c_str(), selected))
                        {
                            cube.targetSpawn = spawnId;
                            MarkSceneChanged(context);
                        }

                        if (selected)
                            ImGui::SetItemDefaultFocus();
                    }

                    ImGui::EndCombo();
                }
            }
        }
        if (ImGui::Button("Duplicate Cube"))
            {
                GridCube copy = cube;
                copy.position.x += 1;

                context.scene.cubes.push_back(copy);

                context.selection.type = SelectionType::GridCube;
                context.selection.index = (int)context.scene.cubes.size() - 1;
                MarkSceneChanged(context);
            }

        if (ImGui::Button("Delete Cube"))
            {
                context.scene.cubes.erase(context.scene.cubes.begin() + index);

                context.selection.type = SelectionType::None;
                context.selection.index = -1;

                MarkSceneChanged(context);

                ImGui::End();
                return;
            }
    }

    if (context.selection.type == SelectionType::Entity)
    {
        int index = context.selection.index;

        if (index < 0 || index >= (int)context.scene.entities.size())
        {
            ImGui::Text("Invalid entity selection.");
            ImGui::End();
            return;
        }

        ImGui::Text("Entity JSON");
        std::string currentName = "Unnamed Entity";

        if (context.scene.entities[index].contains("name"))
        {
            currentName = context.scene.entities[index]["name"];
        }

        char nameBuffer[128];
        strncpy(nameBuffer, currentName.c_str(), sizeof(nameBuffer));
        nameBuffer[sizeof(nameBuffer) - 1] = '\0';

        if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
        {
            context.scene.entities[index]["name"] = nameBuffer;
            MarkSceneChanged(context);
        }
        ImGui::TextWrapped("%s", context.scene.entities[index].dump(2).c_str());
    }
    if (context.selection.type == SelectionType::SpawnPoint)
    {
        int targetIndex = context.selection.index;
        int i = 0;

        for (auto& [id, spawn] : context.scene.playerSpawns)
        {
            if (i == targetIndex)
            {
                ImGui::Text("Spawn Point");

                char idBuffer[128];
                strncpy(idBuffer, id.c_str(), sizeof(idBuffer));
                idBuffer[sizeof(idBuffer) - 1] = '\0';

                ImGui::InputText("ID", idBuffer, sizeof(idBuffer));

                if (ImGui::DragFloat("X", &spawn.x, 0.1f) ||
                    ImGui::DragFloat("Y", &spawn.y, 0.1f) ||
                    ImGui::DragFloat("Z", &spawn.z, 0.1f))
                {
                    MarkSceneChanged(context);
                }

                if (ImGui::InputInt("Skin Choice", &spawn.skinChoice))
                {
                    MarkSceneChanged(context);
                    context.previewDirty = true;
                }

                break;
            }

            i++;
        }
    }
    ImGui::End();
}