#include "../EditorPanels.h"
#include "editor/project/ProjectManager.h"

#include "imgui.h"
#include <cstring>

static std::vector<std::string> GetProjectTextureIds(const EditorContext& context)
{
    std::vector<std::string> ids;

    for (const auto& [id, texture] : context.resourceManager.GetAllTextures())
        ids.push_back(id);

    std::sort(ids.begin(), ids.end());
    return ids;
}

static std::string FindCubeModelIdForTexture(const EditorContext& context, const std::string& textureId)
{
    const auto& manifest = context.resourceManager.GetManifestData();

    if (!manifest.contains("cube_models") || !manifest["cube_models"].is_array())
        return "";

    for (const auto& cubeModel : manifest["cube_models"])
    {
        if (cubeModel.value("textureId", "") == textureId)
            return cubeModel.value("id", "");
    }

    return "";
}

static std::string FindTextureIdForCubeModel(const EditorContext& context, const std::string& modelId)
{
    const auto& manifest = context.resourceManager.GetManifestData();

    if (!manifest.contains("cube_models") || !manifest["cube_models"].is_array())
        return "";

    for (const auto& cubeModel : manifest["cube_models"])
    {
        if (cubeModel.value("id", "") == modelId)
            return cubeModel.value("textureId", "");
    }

    return "";
}

static bool DrawStringDropdownWithNone(
    const std::string& label,
    std::string& value,
    const std::vector<std::string>& options
)
{
    bool changed = false;

    const char* preview = value.empty() ? "None" : value.c_str();

    if (ImGui::BeginCombo(label.c_str(), preview))
    {
        if (ImGui::Selectable("None", value.empty()))
        {
            value.clear();
            changed = true;
        }

        for (const std::string& option : options)
        {
            bool selected = value == option;

            if (ImGui::Selectable(option.c_str(), selected))
            {
                value = option;
                changed = true;
            }

            if (selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    return changed;
}

static void DrawTemplateEditor(EditorContext& context, CubeTemplate& cubeTemplate)
{
    char nameBuffer[128];
    strncpy(nameBuffer, cubeTemplate.name.c_str(), sizeof(nameBuffer));
    nameBuffer[sizeof(nameBuffer) - 1] = '\0';

    if (ImGui::InputText("Template Name", nameBuffer, sizeof(nameBuffer)))
    {
        cubeTemplate.name = nameBuffer;
    }

    const char* types[] = { "floor", "wall", "door", "trigger" };

    int currentType = 0;
    for (int i = 0; i < IM_ARRAYSIZE(types); i++)
    {
        if (cubeTemplate.type == types[i])
            currentType = i;
    }

    if (ImGui::Combo("Cube Type", &currentType, types, IM_ARRAYSIZE(types)))
    {
        cubeTemplate.type = types[currentType];

        if (cubeTemplate.type == "floor")
        {
            cubeTemplate.solid = false;
            cubeTemplate.trigger = false;
        }
        else if (cubeTemplate.type == "wall")
        {
            cubeTemplate.solid = true;
            cubeTemplate.trigger = false;
        }
        else if (cubeTemplate.type == "door" || cubeTemplate.type == "trigger")
        {
            cubeTemplate.solid = false;
            cubeTemplate.trigger = true;
        }
    }

    ImGui::Checkbox("Solid", &cubeTemplate.solid);
    ImGui::Checkbox("Trigger", &cubeTemplate.trigger);
    ImGui::Checkbox("Visible In Game", &cubeTemplate.visible);

    std::string currentTextureId = FindTextureIdForCubeModel(context, cubeTemplate.modelID);
    std::vector<std::string> textureIds = GetProjectTextureIds(context);

    if (DrawStringDropdownWithNone("Texture ID", currentTextureId, textureIds))
    {
        std::string matchingModelId = FindCubeModelIdForTexture(context, currentTextureId);

        if (!matchingModelId.empty())
        {
            cubeTemplate.modelID = matchingModelId;
        }
        else
        {
            cubeTemplate.modelID.clear();
            std::cout << "[GridPainter] No cube model found for texture: " << currentTextureId << "\n";
        }
    }

    char targetSceneBuffer[256];
    strncpy(targetSceneBuffer, cubeTemplate.targetScene.c_str(), sizeof(targetSceneBuffer));
    targetSceneBuffer[sizeof(targetSceneBuffer) - 1] = '\0';

    if (ImGui::InputText("Target Scene", targetSceneBuffer, sizeof(targetSceneBuffer)))
    {
        cubeTemplate.targetScene = targetSceneBuffer;
    }

    char targetSpawnBuffer[128];
    strncpy(targetSpawnBuffer, cubeTemplate.targetSpawn.c_str(), sizeof(targetSpawnBuffer));
    targetSpawnBuffer[sizeof(targetSpawnBuffer) - 1] = '\0';

    if (ImGui::InputText("Target Spawn", targetSpawnBuffer, sizeof(targetSpawnBuffer)))
    {
        cubeTemplate.targetSpawn = targetSpawnBuffer;
    }
}

void DrawGridPainter(EditorContext& context)
{
    ImGui::Begin("Grid Painter");

    ImGui::Checkbox("Enable Grid Painter", &context.gridPainterEnabled);
    if (context.gridPainterUndoStack.empty())
    {
        ImGui::BeginDisabled();
    }

    if (ImGui::Button("Undo Last Paint Edit"))
    {
        UndoGridPainterAction(context);
    }

    if (context.gridPainterUndoStack.empty())
    {
        ImGui::EndDisabled();
    }

    ImGui::SameLine();
    ImGui::TextDisabled("Ctrl+Z");
    ImGui::InputInt("Y Layer", &context.gridPainterLayerY);

    ImGui::SeparatorText("Mode");

    int mode = (int)context.gridPainterMode;

    if (ImGui::RadioButton("Paint", mode == (int)GridPainterMode::Paint))
        context.gridPainterMode = GridPainterMode::Paint;

    ImGui::SameLine();

    if (ImGui::RadioButton("Erase", mode == (int)GridPainterMode::Erase))
        context.gridPainterMode = GridPainterMode::Erase;

    ImGui::SameLine();

    if (ImGui::RadioButton("Eyedropper", mode == (int)GridPainterMode::Eyedropper))
        context.gridPainterMode = GridPainterMode::Eyedropper;

    ImGui::SeparatorText("Templates");

    for (int i = 0; i < 5; i++)
    {
        ImGui::PushID(i);

        bool selected = context.selectedCubeTemplate == i;

        if (ImGui::Selectable(context.cubeTemplates[i].name.c_str(), selected))
        {
            context.selectedCubeTemplate = i;
        }

        ImGui::PopID();
    }

    ImGui::SeparatorText("Selected Template");

    CubeTemplate& selectedTemplate =
        context.cubeTemplates[context.selectedCubeTemplate];

    DrawTemplateEditor(context, selectedTemplate);

    ImGui::Separator();

    if (context.selection.type == SelectionType::GridCube)
    {
        int index = context.selection.index;

        if (index >= 0 && index < (int)context.scene.cubes.size())
        {
            if (ImGui::Button("Save Selected Cube To Template"))
            {
                CopyCubeToTemplate(context, context.scene.cubes[index]);
            }
        }
    }
    else
    {
        ImGui::TextDisabled("Select a cube to save it as a template.");
    }
    if (ImGui::Button("Save Templates"))
    {
        SaveEditorSettings(context);
    }
    ImGui::End();
}