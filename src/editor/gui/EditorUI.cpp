#include "EditorUI.h"
#include "imgui.h"

void SetupEditorStyle()
{
    ImGui::StyleColorsDark();
}

void DrawEditorUI()
{
    ImGui::Begin("Assets");
    ImGui::Button("Cube");
    ImGui::Button("NPC");
    ImGui::End();

    ImGui::Begin("Viewport");
    ImGui::Text("Scene view here");
    ImGui::End();

    ImGui::Begin("Inspector");
    ImGui::Text("Selected Object: Door");
    ImGui::End();
}