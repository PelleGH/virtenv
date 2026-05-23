#include "EditorPanels.h"
#include "imgui.h"

void DrawHierarchy(EditorContext& context)
{
    ImGui::Begin("Hierarchy");
    if (context.scenePaths.empty()) {
        ImGui::Text("No scenes available.");
        ImGui::End();
        return;
    }
    

    if (!context.sceneLoaded)
    {
        ImGui::Text("No scene loaded.");
        ImGui::End();
        return;
    }

    ImGui::Text("Scene");
    ImGui::SameLine();

    const char* currentName = context.scenePaths[context.currentSceneIndex].c_str();

    if (ImGui::BeginCombo("##SceneSelector", currentName))
    {
        for (int i = 0; i < (int)context.scenePaths.size(); i++)
        {
            bool selected = i == context.currentSceneIndex;

            if (ImGui::Selectable(context.scenePaths[i].c_str(), selected))
            {
                LoadEditorScene(context, i);
            }

            if (selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    if (ImGui::TreeNode("Static Grid Cubes"))
    {
        for (int i = 0; i < (int)context.scene.cubes.size(); i++)
        {
            const auto& cube = context.scene.cubes[i];

            char label[128];
            snprintf(label, sizeof(label), "%s (%d, %d, %d)##cube_%d",
                cube.type.c_str(),
                cube.position.x,
                cube.position.y,
                cube.position.z,
                i
            );

            bool selected =
                context.selection.type == SelectionType::GridCube &&
                context.selection.index == i;

            if (ImGui::Selectable(label, selected))
            {
                context.selection.type = SelectionType::GridCube;
                context.selection.index = i;
            }
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Entities"))
    {
        for (int i = 0; i < (int)context.scene.entities.size(); i++)
        {
            auto& entity = context.scene.entities[i];

            std::string displayName;

            if (entity.contains("name") && entity["name"].is_string())
            {
                displayName = entity["name"];
            }
            else
            {
                displayName = "Unnamed Entity";
            }

            char label[128];

            snprintf(label,
                sizeof(label),
                "%s##entity_%d",
                displayName.c_str(),
                i
            );

            bool selected =
                context.selection.type == SelectionType::Entity &&
                context.selection.index == i;

            if (ImGui::Selectable(label, selected))
            {
                context.selection.type = SelectionType::Entity;
                context.selection.index = i;
            }
        }

        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Spawn Points"))
        {
            int i = 0;

            for (auto& [id, spawn] : context.scene.playerSpawns)
            {
                char label[128];
                snprintf(label, sizeof(label), "%s##spawn_%d", id.c_str(), i);

                bool selected =
                    context.selection.type == SelectionType::SpawnPoint &&
                    context.selection.index == i;

                if (ImGui::Selectable(label, selected))
                {
                    context.selection.type = SelectionType::SpawnPoint;
                    context.selection.index = i;
                }

                i++;
            }

            ImGui::TreePop();
        }

    ImGui::End();
}