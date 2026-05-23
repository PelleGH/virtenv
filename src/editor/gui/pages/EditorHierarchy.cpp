#include "EditorPanels.h"
#include "imgui.h"
#include <string>

void DrawHierarchy(EditorContext& context)
{
    // To remember the state for text box per frame
    static int renamingEntityIndex = -1;            // Indicates that there is no changes
    static char renameEntityBuffer[128] = "";       // Buffer meanwhile the user is writing

    if (context.scenePaths.empty()) {
        ImGui::Text("No scenes available.");
        ImGui::End();
        return;
    }
    ImGui::Begin("Hierarchy");

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

    ImGui::Separator();

    // Begins the TabBar system
    if (ImGui::BeginTabBar("HierarchyTabs")) {

        if (ImGui::BeginTabItem("Static Grid Cubes"))
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
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Entities"))
        {
            for (int i = 0; i < (int)context.scene.entities.size(); i++)
            {
                auto& entity = context.scene.entities[i];

                // Reads name from JSON, if empty show "Unamed Entity"
                std::string displayName = (entity.contains("name") && entity["name"].is_string()) ? entity["name"] : "Unamed Entity";

                // Gives each row a ID to seperate the textboxes
                ImGui::PushID(i + 2000);

                // Renaming mode (double click)
                if (renamingEntityIndex == i) {
                    
                    ImGui::SetKeyboardFocusHere();

                    // Renders the input and condition is done when pressing "Enter" or click outside the textbox
                    if (ImGui::InputText("##renameEntity", renameEntityBuffer, sizeof(renameEntityBuffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll) || (ImGui::IsItemDeactivated() && ImGui::IsWindowFocused())) {
                        
                        // Save buffer to JSON data
                        entity["name"] = std::string(renameEntityBuffer);
                        renamingEntityIndex = -1;
                    }

                }else{
                    // View mode (single click)

                    char label[128];
                    snprintf(label, sizeof(label), "%s##entity_%d", displayName.c_str(), i);

                    bool selected = context.selection.type == SelectionType::Entity && context.selection.index == i;

                    // Renders the row and mark it as selected
                    if (ImGui::Selectable(label, selected)) {
                        context.selection.type = SelectionType::Entity;
                        context.selection.index = i;
                    }

                    // Listener for double click, 
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                        renamingEntityIndex = i;

                        // Prefill the texbox with current name when exit
                        strncpy(renameEntityBuffer, displayName.c_str(), sizeof(renameEntityBuffer));
                    }
                }
                // Close the ID
                ImGui::PopID();
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Spawn Points"))
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
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}
    

