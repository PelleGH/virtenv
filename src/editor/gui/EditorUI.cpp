#include "EditorUI.h"

#include "editor/project/ProjectManager.h"
#include "engine/scene/SceneLoader.h"

#include "imgui.h"

#include <iostream>
#include <cstring>
#include <cstdlib>

void SetupEditorStyle()
{
    ImGui::StyleColorsDark();
}

static void DrawHierarchy(EditorContext& context)
{
    ImGui::Begin("Hierarchy");

    if (!context.sceneLoaded)
    {
        ImGui::Text("No scene loaded.");
        ImGui::End();
        return;
    }

    ImGui::Text("Scene: %s", context.scene.name.c_str());

    if (ImGui::TreeNode("Static Grid Cubes"))
    {
        for (int i = 0; i < (int)context.scene.cubes.size(); i++)
        {
            const auto& cube = context.scene.cubes[i];

            char label[128];
            snprintf(label, sizeof(label), "%s (%d, %d, %d)",
                cube.type.c_str(),
                cube.position.x,
                cube.position.y,
                cube.position.z
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

    ImGui::End();
}

static void DrawInspector(EditorContext& context)
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

        if (ImGui::InputInt("X", &cube.position.x)) context.dirty = true;
        if (ImGui::InputInt("Y", &cube.position.y)) context.dirty = true;
        if (ImGui::InputInt("Z", &cube.position.z)) context.dirty = true;

        const char* types[] = { "floor", "wall", "door", "trigger" };

        int currentType = 0;

        for (int i = 0; i < 4; i++)
        {
            if (cube.type == types[i])
                currentType = i;
        }

        if (ImGui::Combo("Type", &currentType, types, IM_ARRAYSIZE(types)))
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
            else if (cube.type == "trigger")
            {
                cube.solid = false;
                cube.trigger = true;
            }

            context.dirty = true;
        }

        if (ImGui::Checkbox("Solid", &cube.solid)) context.dirty = true;
        if (ImGui::Checkbox("Trigger", &cube.trigger)) context.dirty = true;
        if (ImGui::Button("Duplicate Cube"))
            {
                GridCube copy = cube;
                copy.position.x += 1;

                context.scene.cubes.push_back(copy);

                context.selection.type = SelectionType::GridCube;
                context.selection.index = (int)context.scene.cubes.size() - 1;

                context.dirty = true;
            }

            if (ImGui::Button("Delete Cube"))
            {
                context.scene.cubes.erase(context.scene.cubes.begin() + index);

                context.selection.type = SelectionType::None;
                context.selection.index = -1;

                context.dirty = true;

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
            context.dirty = true;
        }
        ImGui::TextWrapped("%s", context.scene.entities[index].dump(2).c_str());
    }

    ImGui::End();
}
static void DrawDockspace(EditorContext& context)
{
    static bool dockspaceOpen = true;

    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoBackground;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("Editor Dockspace", &dockspaceOpen, windowFlags);

    ImGui::PopStyleVar(3);
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ImGui::TextDisabled("Use Project Launcher to Load");

            if (ImGui::MenuItem("Save Project", "Ctrl+Shift+S"))
            {
                SaveProject(context);
            }

            ImGui::Separator();

            //SAVE BUTTON
            if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
            {
                if (SceneLoader::saveToFile(context.currentScenePath, context.scene)) {
                    std::cout << "[Editor] Successfully saved to " << context.currentScenePath << std::endl;
                    context.dirty = false;
                } else {
                    std::cout << "[Editor] ERROR: Failed to save scene!"<< std::endl;
                }
            }

            //LOAD BUTTON
            if (ImGui::MenuItem("Load Scene", "Ctrl+O"))
            {
                if (SceneLoader::loadFromFile(context.currentScenePath, context.scene)) {
                    std::cout << "[Editor] Successfully loaded " << context.currentScenePath << std::endl;
                    context.sceneLoaded = true;
                } else {
                    std::cout << "[Editor] ERROR: Failed to load scene!"<< std::endl;
                }
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit"))
            {
                exit(0);
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            ImGui::MenuItem("Undo", "Ctrl+Z", false, false);
            ImGui::MenuItem("Redo", "Ctrl+Y", false, false);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Hierarchy");
            ImGui::MenuItem("Inspector");
            ImGui::MenuItem("Assets");
            ImGui::MenuItem("Viewport");
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGuiID dockspaceID = ImGui::GetID("MainDockspace");
    ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f));

    ImGui::End();
}

void DrawProjectLauncher(EditorContext& context) {
    if (context.availableProjects.empty()) {
        RefreshProjectList(context);
    }

    ImGui::Begin("Project Launcher");
    
    ImGui::Text("Active Project: %s", context.projectName.c_str());
    if (ImGui::Button("Refresh Directory List")) {
        RefreshProjectList(context);
    }
    
    ImGui::Separator();
    
    ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Create New Game Project:");
    
    static char folderNameBuf[64] = "";
    static char projNameBuf[128] = "";

    ImGui::InputText("Folder Name (e.g. MyRPG)", folderNameBuf, IM_ARRAYSIZE(folderNameBuf));
    ImGui::InputText("Display Game Name", projNameBuf, IM_ARRAYSIZE(projNameBuf));

    if (ImGui::Button("Generate Project Templates")) {
        // Run our automated file generation routine!
        CreateNewProject(context, folderNameBuf, projNameBuf);
        
        // Clear out the text inputs so they are ready for next time
        std::memset(folderNameBuf, 0, sizeof(folderNameBuf));
        std::memset(projNameBuf, 0, sizeof(projNameBuf));
    }
    // --------------------------------------------------

    ImGui::Separator();
    ImGui::Text("Available Game Projects:");

    for (const auto& projName : context.availableProjects) {
        ImGui::BulletText("%s", projName.c_str());
        ImGui::SameLine(ImGui::GetWindowWidth() - 80);
        
        char buttonLabel[64];
        snprintf(buttonLabel, sizeof(buttonLabel), "Open##%s", projName.c_str());
        
        if (ImGui::Button(buttonLabel)) {
            LoadProject(context, projName); 
        }
    }

    ImGui::End();
}

void DrawEditorUI(EditorContext& context)
{
    DrawDockspace(context);
    ImGui::Begin("Assets");
    if (ImGui::Button("Add Floor Cube"))
        {
            GridCube cube;
            cube.type = "floor";
            cube.position = {0, 0, 0};
            cube.solid = false;
            cube.trigger = false;

            context.scene.cubes.push_back(cube);

            context.selection.type = SelectionType::GridCube;
            context.selection.index = (int)context.scene.cubes.size() - 1;

            context.dirty = true;
        }

        if (ImGui::Button("Add Wall Cube"))
        {
            GridCube cube;
            cube.type = "wall";
            cube.position = {0, 0, 0};
            cube.solid = true;
            cube.trigger = false;

            context.scene.cubes.push_back(cube);

            context.selection.type = SelectionType::GridCube;
            context.selection.index = (int)context.scene.cubes.size() - 1;

            context.dirty = true;
        }

    ImGui::Button("NPC");
    ImGui::End();

    ImGui::Begin("Viewport");
    ImGui::Text("Scene view later.");
    ImGui::Text("Cubes loaded: %d", (int)context.scene.cubes.size());
    ImGui::Text("Entities loaded: %d", (int)context.scene.entities.size());
    ImGui::End();

    DrawHierarchy(context);
    DrawInspector(context);
    DrawProjectLauncher(context);
}