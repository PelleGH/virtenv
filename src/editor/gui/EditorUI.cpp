#include "EditorUI.h"

#include "EditorPanels.h"
#include "editor/project/ProjectManager.h"
#include "engine/scene/SceneLoader.h"
#include "pages/ProjectLauncherPanel.h"

#include "imgui.h"
#include "raylib.h"
#include "rlImGui.h"
#include <raymath.h>

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <string>

// Keeps track of open windows
static bool showHierarchy = true;
static bool showInspector = true;
static bool showAssets = true;
static bool showViewport = true;
static bool showProjectLauncher = true;

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

            if (ImGui::MenuItem("Save Project", "Ctrl+S"))
            {
                SaveProject(context);
            }

            ImGui::Separator();

            std::string buildLabel = "Build Game...";
            if (context.buildOutdated)
            {
                buildLabel += " *(Outdated)*"; // Adds it directly to the name
            }

            // 2. Pass the combined string as the FIRST parameter
            if (ImGui::MenuItem(buildLabel.c_str()))
            {
                if (!context.projectPath.empty())
                {
                    std::string outDir = context.projectPath + "Builds/" + context.projectName;
                    BuildProject(context, outDir);
                }
                else
                {
                    std::cout << "Cannot build: No project loaded.\n";
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
        if (ImGui::BeginMenu("Add"))
        {
            if (ImGui::BeginMenu("Cube"))
            {
                if (ImGui::MenuItem("Floor"))
                    AddCube(context, "floor");

                if (ImGui::MenuItem("Wall"))
                    AddCube(context, "wall");

                if (ImGui::MenuItem("Door"))
                    AddCube(context, "door");

                if (ImGui::MenuItem("Trigger"))
                    AddCube(context, "trigger");

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Entity"))
            {
                if (ImGui::MenuItem("NPC"))
                {
                    // Add entity later
                }

                if (ImGui::MenuItem("Enemy"))
                {
                    // Add entity later
                }

                if (ImGui::MenuItem("Pickup"))
                {
                    // Add entity later
                }

                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Spawn Point"))
                AddSpawnPoint(context);

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {
            // Gives the button check-marks
            ImGui::MenuItem("Hierarchy", NULL, &showHierarchy);
            ImGui::MenuItem("Inspector", NULL, &showInspector);
            ImGui::MenuItem("Assets", NULL, &showAssets);
            ImGui::MenuItem("Viewport", NULL, &showViewport);
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGuiID dockspaceID = ImGui::GetID("MainDockspace");
    ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f));

    ImGui::End();
}



void DrawEditorUI(EditorContext& context)
{
    DrawDockspace(context);

    // If check-marks in meny, draw
    if (showAssets){
        DrawAssets(context);
    }

    if (showViewport){
        DrawViewport(context);
    }

    if (showHierarchy){
        DrawHierarchy(context);
    }

    if (showInspector){
        DrawInspector(context);
    }

    if (showProjectLauncher){
        DrawProjectLauncher(context);
    }
}