#include "EditorUI.h"
#include "imgui.h"
#include "raylib.h"
#include "rlImGui.h"
#include <cstring>
#include "../../engine/scene/SceneLoader.h"
#include <raymath.h>
#include "EditorPanels.h"

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
            if (ImGui::MenuItem("Save Scene"))
            {
                // call save here later
            }

            if (ImGui::MenuItem("Load Scene"))
            {
                // call load here later
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit"))
            {
                // handle exit later
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
void DrawEditorUI(EditorContext& context)
{
    DrawDockspace(context);

    DrawAssets(context);
    DrawViewport(context);
    DrawHierarchy(context);
    DrawInspector(context);
}