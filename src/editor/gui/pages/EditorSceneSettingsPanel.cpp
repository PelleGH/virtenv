#include "editor/gui/pages/EditorSceneSettingsPanel.h"

#include "editor/gui/EditorPanels.h"
#include "imgui.h"

#include <cstring>

void DrawSceneSettingsPanel(EditorContext& context)
{
    if (!context.sceneLoaded)
        return;

    ImGui::Begin("Scene Settings");
    ImGui::SeparatorText("Project");

    const char* currentStartingScene =
        context.startingScene.empty() ? "None" : context.startingScene.c_str();

    if (ImGui::BeginCombo("Starting Scene", currentStartingScene))
    {
        for (const auto& scenePath : context.scenePaths)
        {
            std::string sceneName = std::filesystem::path(scenePath).stem().string();
            bool selected = context.startingScene == sceneName;

            if (ImGui::Selectable(sceneName.c_str(), selected))
            {
                context.startingScene = sceneName;
                context.dirty = true;
            }

            if (selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ImGui::Text("Scene");
    ImGui::Separator();

    char sceneNameBuffer[128];
    std::strncpy(sceneNameBuffer, context.scene.name.c_str(), sizeof(sceneNameBuffer));
    sceneNameBuffer[sizeof(sceneNameBuffer) - 1] = '\0';

    if (ImGui::InputText("Scene Name", sceneNameBuffer, sizeof(sceneNameBuffer)))
    {
        context.scene.name = sceneNameBuffer;
        MarkSceneChanged(context);
    }

    ImGui::Spacing();
    ImGui::Text("Camera");
    ImGui::Separator();

    const char* modes[] = { "fixed", "followPlayer" };
    int currentMode = context.scene.camera.mode == "fixed" ? 0 : 1;

    if (ImGui::Combo("Mode", &currentMode, modes, IM_ARRAYSIZE(modes)))
    {
        context.scene.camera.mode = modes[currentMode];
        MarkSceneChanged(context);
    }

    if (context.scene.camera.mode == "fixed")
    {
        ImGui::TextDisabled("Fixed camera uses a world position and target.");

        if (ImGui::DragFloat("Position X", &context.scene.camera.positionX, 0.1f) ||
            ImGui::DragFloat("Position Y", &context.scene.camera.positionY, 0.1f) ||
            ImGui::DragFloat("Position Z", &context.scene.camera.positionZ, 0.1f) ||
            ImGui::DragFloat("Target X", &context.scene.camera.targetX, 0.1f) ||
            ImGui::DragFloat("Target Y", &context.scene.camera.targetY, 0.1f) ||
            ImGui::DragFloat("Target Z", &context.scene.camera.targetZ, 0.1f))
        {
            MarkSceneChanged(context);
        }
    }
    else
    {
        ImGui::TextDisabled("Follow player uses position as an offset from the player.");

        if (ImGui::DragFloat("Offset X", &context.scene.camera.positionX, 0.1f) ||
            ImGui::DragFloat("Offset Y", &context.scene.camera.positionY, 0.1f) ||
            ImGui::DragFloat("Offset Z", &context.scene.camera.positionZ, 0.1f))
        {
            MarkSceneChanged(context);
        }
    }

    ImGui::End();
}