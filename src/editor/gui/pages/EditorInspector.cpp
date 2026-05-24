#include "../EditorPanels.h"
#include "imgui.h"
#include "../../../engine/scene/SceneLoader.h"

#include <cstring>
#include <vector>
#include <string>
#include <unordered_set>
#include <algorithm>

static std::string HiddenId(const std::string& label);
static void DrawFieldLabel(const std::string& label);
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
static bool DrawStringDropdown(
    const std::string& label,
    nlohmann::json& value,
    const std::vector<std::string>& options
)
{
    bool changed = false;

    if (!value.is_string())
    {
        value = options.empty() ? "" : options[0];
        changed = true;
    }

    std::string current = value.get<std::string>();

    DrawFieldLabel(label);

    std::string id = HiddenId(label);

    if (ImGui::BeginCombo(id.c_str(), current.c_str()))
    {
        for (const std::string& option : options)
        {
            bool selected = (current == option);

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
static bool DrawJsonColorField(const std::string& label, nlohmann::json& value)
{
    if (!value.is_array() || value.size() != 4)
        return false;

    int rgba[4] = {255, 255, 255, 255};

    for (int i = 0; i < 4; i++)
    {
        if (value[i].is_number_integer())
            rgba[i] = value[i].get<int>();
    }

    DrawFieldLabel(label + " rgba");

    std::string id = HiddenId(label);

    if (ImGui::InputInt4(id.c_str(), rgba))
    {
        for (int i = 0; i < 4; i++)
            rgba[i] = std::clamp(rgba[i], 0, 255);

        value = nlohmann::json::array({ rgba[0], rgba[1], rgba[2], rgba[3] });
        return true;
    }

    return false;
}

static bool DrawJsonField(
    EditorContext& context,
    const std::string& label,
    nlohmann::json& value,
    const std::string& typeOverride = ""
);

static std::string GetFieldTypeOverride(
    const EditorContext& context,
    const std::string& componentName,
    const std::string& fieldName
)
{
    if (!context.componentSchemas.contains(componentName))
        return "";

    const auto& schema = context.componentSchemas[componentName];

    if (!schema.contains("fieldTypes") || !schema["fieldTypes"].is_object())
        return "";

    const auto& fieldTypes = schema["fieldTypes"];

    if (!fieldTypes.contains(fieldName) || !fieldTypes[fieldName].is_string())
        return "";

    return fieldTypes[fieldName].get<std::string>();
}
static nlohmann::json GetComponentDefaults(
    const EditorContext& context,
    const std::string& componentName
)
{
    if (!context.componentSchemas.contains(componentName))
        return nlohmann::json::object();

    const auto& schema = context.componentSchemas[componentName];

    if (!schema.contains("default") || !schema["default"].is_object())
        return nlohmann::json::object();

    return schema["default"];
}

static std::vector<std::string> GetComponentFieldOrder(
    const EditorContext& context,
    const std::string& componentName
)
{
    std::vector<std::string> result;

    if (!context.componentSchemas.contains(componentName))
        return result;

    const auto& schema = context.componentSchemas[componentName];

    if (!schema.contains("fieldOrder") || !schema["fieldOrder"].is_array())
        return result;

    for (const auto& field : schema["fieldOrder"])
    {
        if (field.is_string())
            result.push_back(field.get<std::string>());
    }

    return result;
}

static nlohmann::json MakeVisibleComponentData(
    const EditorContext& context,
    const std::string& componentName,
    const nlohmann::json& componentData
)
{
    nlohmann::json visible = GetComponentDefaults(context, componentName);

    if (!visible.is_object())
        visible = nlohmann::json::object();

    if (componentData.is_object())
    {
        for (auto& [key, value] : componentData.items())
        {
            visible[key] = value;
        }
    }

    return visible;
}

static int GetComponentOrder(
    const EditorContext& context,
    const std::string& componentName
)
{
    if (!context.componentSchemas.contains(componentName))
    {
        return 10000;
    }

    const auto& schema = context.componentSchemas[componentName];

    if (!schema.contains("order") || !schema["order"].is_number_integer())
    {
        return 10000;
    }

    return schema["order"].get<int>();
}
static std::string HiddenId(const std::string& label)
{
    return "##" + label;
}
static void DrawFieldLabel(const std::string& label)
{
    constexpr float labelWidth = 120.0f;

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label.c_str());
    ImGui::SameLine(labelWidth);
    ImGui::SetNextItemWidth(-1.0f);
}
static bool IsFieldHidden(
    const EditorContext& context,
    const std::string& componentName,
    const std::string& fieldName
)
{
    if (!context.componentSchemas.contains(componentName))
        return false;

    const auto& schema = context.componentSchemas[componentName];

    if (!schema.contains("hiddenFields") || !schema["hiddenFields"].is_array())
        return false;

    for (const auto& hiddenField : schema["hiddenFields"])
    {
        if (hiddenField.is_string() && hiddenField.get<std::string>() == fieldName)
            return true;
    }

    return false;
}

static bool DrawJsonStringField(const std::string& label, nlohmann::json& value)
{
    std::string current = value.get<std::string>();

    char buffer[256];
    std::strncpy(buffer, current.c_str(), sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';

    DrawFieldLabel(label);

    std::string id = HiddenId(label);

    if (ImGui::InputText(id.c_str(), buffer, sizeof(buffer)))
    {
        value = std::string(buffer);
        return true;
    }

    return false;
}

static bool DrawJsonActionObject(EditorContext& context, int index, nlohmann::json& action)
{
    bool changed = false;

    if (!action.is_object())
    {
        action = {
            {"type", ""},
            {"target", ""}
        };
        changed = true;
    }

    std::string header = "Action " + std::to_string(index);

    if (!ImGui::TreeNodeEx(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        return changed;

    ImGui::Indent();

    if (!action.contains("type"))
    {
        action["type"] = "";
        changed = true;
    }

    if (!action.contains("target"))
    {
        action["target"] = "";
        changed = true;
    }

    static const std::vector<std::string> actionTypes = {
        "PickupItem",
        "StartDialogue",
        "SceneTransition",
        "SpawnEntity"
    };

    if (DrawStringDropdown("type", action["type"], actionTypes))
        changed = true;

    if (DrawJsonField(context, "target", action["target"]))
        changed = true;

    ImGui::Unindent();
    ImGui::TreePop();

    return changed;
}

static bool DrawJsonActionsArray(EditorContext& context, nlohmann::json& value)
{
    bool changed = false;

    if (!value.is_array())
    {
        value = nlohmann::json::array();
        changed = true;
    }

    if (!ImGui::TreeNodeEx("actions", ImGuiTreeNodeFlags_DefaultOpen))
        return changed;

    ImGui::Indent();

    for (int i = 0; i < static_cast<int>(value.size()); i++)
    {
        ImGui::PushID(i);

        if (DrawJsonActionObject(context, i, value[i]))
            changed = true;

        if (ImGui::Button("Remove Action"))
        {
            value.erase(value.begin() + i);
            changed = true;
            ImGui::PopID();
            break;
        }

        ImGui::Separator();
        ImGui::PopID();
    }

    if (ImGui::Button("Add Action"))
    {
        value.push_back({
            {"type", "PickupItem"},
            {"target", ""}
        });

        changed = true;
    }

    ImGui::Unindent();
    ImGui::TreePop();

    return changed;
}

static bool DrawJsonArray(EditorContext& context, const std::string& label, nlohmann::json& value)
{
    if (label == "actions")
    {
        return DrawJsonActionsArray(context, value);
    }

    bool changed = false;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;

    if (!ImGui::TreeNodeEx(label.c_str(), flags))
        return false;

    ImGui::Indent();

    for (int i = 0; i < static_cast<int>(value.size()); i++)
    {
        ImGui::PushID(i);

        std::string itemLabel = "Item " + std::to_string(i);

        if (DrawJsonField(context, itemLabel, value[i]))
            changed = true;

        if (ImGui::Button("Remove Item"))
        {
            value.erase(value.begin() + i);
            changed = true;
            ImGui::PopID();
            break;
        }

        ImGui::Separator();
        ImGui::PopID();
    }

    if (ImGui::Button("Add String Item"))
    {
        value.push_back("");
        changed = true;
    }

    ImGui::Unindent();
    ImGui::TreePop();

    return changed;
}

static bool DrawJsonObject(EditorContext& context, const std::string& label, nlohmann::json& value)
{
    bool changed = false;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;

    if (!ImGui::TreeNodeEx(label.c_str(), flags))
        return false;

    ImGui::Indent();

    for (auto& [fieldName, fieldValue] : value.items())
    {
        ImGui::PushID(fieldName.c_str());

        if (DrawJsonField(context, fieldName, fieldValue))
            changed = true;

        ImGui::PopID();
        ImGui::Spacing();
    }

    ImGui::Unindent();
    ImGui::TreePop();

    return changed;
}

static bool DrawJsonField(
    EditorContext& context,
    const std::string& label,
    nlohmann::json& value,
    const std::string& typeOverride
)
{
    std::string id = HiddenId(label);
    if (label == "color" && value.is_array())
    {
        return DrawJsonColorField(label, value);
    }
    if (typeOverride == "float" && value.is_number())
    {
        float current = value.get<float>();

        DrawFieldLabel(label);

        if (ImGui::InputFloat(id.c_str(), &current, 0.1f, 1.0f, "%.3f"))
        {
            value = current;
            return true;
        }

        return false;
    }
    if (value.is_boolean())
    {
        bool current = value.get<bool>();

        DrawFieldLabel(label);

        if (ImGui::Checkbox(id.c_str(), &current))
        {
            value = current;
            return true;
        }

        return false;
    }

    if (value.is_number_integer())
    {
        int current = value.get<int>();

        DrawFieldLabel(label);

        if (ImGui::InputInt(id.c_str(), &current))
        {
            value = current;
            return true;
        }

        return false;
    }

    if (value.is_number_unsigned())
    {
        int current = static_cast<int>(value.get<unsigned int>());

        DrawFieldLabel(label);

        if (ImGui::InputInt(id.c_str(), &current))
        {
            if (current < 0)
                current = 0;

            value = static_cast<unsigned int>(current);
            return true;
        }

        return false;
    }

    if (value.is_number_float())
    {
        float current = value.get<float>();

        DrawFieldLabel(label);

        if (ImGui::InputFloat(id.c_str(), &current, 0.1f, 1.0f, "%.3f"))
        {
            value = current;
            return true;
        }

        return false;
    }

    if (value.is_string())
    {
        return DrawJsonStringField(label, value);
    }

    if (value.is_array())
    {
        return DrawJsonArray(context, label, value);
    }

    if (value.is_object())
    {
        return DrawJsonObject(context, label, value);
    }

    if (value.is_null())
    {
        ImGui::TextDisabled("%s: null", label.c_str());
        return false;
    }

    ImGui::TextDisabled("%s: unsupported value", label.c_str());
    return false;
}

static void DrawJsonComponent(
    EditorContext& context,
    const std::string& componentName,
    nlohmann::json& componentData
)
{
    if (!componentData.is_object())
        return;

    ImGui::PushID(componentName.c_str());

    bool open = ImGui::CollapsingHeader(
        componentName.c_str(),
        ImGuiTreeNodeFlags_DefaultOpen
    );

    if (!open)
    {
        ImGui::PopID();
        return;
    }

    bool changed = false;

    nlohmann::json visibleData = MakeVisibleComponentData(context, componentName, componentData);

    std::vector<std::string> fieldOrder = GetComponentFieldOrder(context, componentName);
    std::unordered_set<std::string> drawnFields;

    for (const std::string& fieldName : fieldOrder)
    {
        if (!visibleData.contains(fieldName))
            continue;

        if (IsFieldHidden(context, componentName, fieldName))
            continue;

        ImGui::PushID(fieldName.c_str());

        std::string typeOverride = GetFieldTypeOverride(context, componentName, fieldName);

        if (DrawJsonField(context, fieldName, visibleData[fieldName], typeOverride))
            changed = true;

        drawnFields.insert(fieldName);

        ImGui::PopID();
        ImGui::Spacing();
    }

    for (auto& [fieldName, fieldValue] : visibleData.items())
    {
        if (drawnFields.find(fieldName) != drawnFields.end())
            continue;

        if (IsFieldHidden(context, componentName, fieldName))
            continue;

        ImGui::PushID(fieldName.c_str());

        std::string typeOverride = GetFieldTypeOverride(context, componentName, fieldName);

        if (DrawJsonField(context, fieldName, fieldValue, typeOverride))
            changed = true;

        ImGui::PopID();
        ImGui::Spacing();
    }

    if (changed)
    {
        componentData = visibleData;
        MarkSceneChanged(context);
    }

    ImGui::Spacing();

    std::string removeButton = "Remove " + componentName;

    if (ImGui::Button(removeButton.c_str()))
    {
        componentData = nullptr;
        MarkSceneChanged(context);
    }

    ImGui::PopID();
}

static void DrawAddComponentDropdown(EditorContext& context, nlohmann::json& entity)
{
    if (!context.componentSchemas.is_object() || context.componentSchemas.empty())
    {
        ImGui::TextDisabled("No component schemas loaded.");
        return;
    }

    if (ImGui::BeginCombo("Add Component", "Choose component"))
    {
        for (auto& [componentName, schema] : context.componentSchemas.items())
        {
            if (entity.contains(componentName))
                continue;

            if (!schema.contains("default"))
                continue;

            if (ImGui::Selectable(componentName.c_str()))
            {
                entity[componentName] = schema["default"];
                MarkSceneChanged(context);
            }
        }

        ImGui::EndCombo();
    }
}
void DrawInspector(EditorContext& context)
{
    ImGui::Begin("Inspector");
    if (context.componentSchemas.empty())
    {
        LoadComponentSchemas(context);
    }
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
        if (ImGui::Checkbox("Visible In Game", &cube.visible)) MarkSceneChanged(context);
        
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
                    std::string sceneName = std::filesystem::path(scenePath).stem().string();

                    bool selected = cube.targetScene == sceneName;

                    if (ImGui::Selectable(sceneName.c_str(), selected))
                    {
                        cube.targetScene = sceneName;
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
                std::string targetScenePath;

                for (const auto& scenePath : context.scenePaths)
                {
                    if (std::filesystem::path(scenePath).stem().string() == cube.targetScene)
                    {
                        targetScenePath = scenePath;
                        break;
                    }
                }

                std::vector<std::string> spawnIds;

                if (!targetScenePath.empty())
                {
                    spawnIds = GetSpawnIdsForScene(targetScenePath);
                }

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

    auto& entity = context.scene.entities[index];

    ImGui::Text("Entity");

    std::string currentName = entity.value("name", "Unnamed Entity");

    char nameBuffer[128];
    std::strncpy(nameBuffer, currentName.c_str(), sizeof(nameBuffer));
    nameBuffer[sizeof(nameBuffer) - 1] = '\0';

    if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
    {
        entity["name"] = nameBuffer;
        MarkSceneChanged(context);
    }

    ImGui::SeparatorText("Components");

    std::vector<std::string> componentNames;

    for (auto& [componentName, componentData] : entity.items())
    {
        if (componentName == "name" || componentName == "id")
            continue;

        componentNames.push_back(componentName);
    }

    std::sort(
        componentNames.begin(),
        componentNames.end(),
        [&context](const std::string& a, const std::string& b)
        {
            int orderA = GetComponentOrder(context, a);
            int orderB = GetComponentOrder(context, b);

            if (orderA != orderB)
                return orderA < orderB;

            return a < b;
        }
    );

    std::vector<std::string> componentsToRemove;

    for (const auto& componentName : componentNames)
    {
        auto& componentData = entity[componentName];

        DrawJsonComponent(context, componentName, componentData);

        if (componentData.is_null())
            componentsToRemove.push_back(componentName);
    }

    for (const auto& componentName : componentsToRemove)
    {
        entity.erase(componentName);
    }

    ImGui::SeparatorText("Add Component");
    DrawAddComponentDropdown(context, entity);

    ImGui::SeparatorText("Debug");

    if (ImGui::TreeNode("Raw JSON"))
    {
        ImGui::TextWrapped("%s", entity.dump(2).c_str());
        ImGui::TreePop();
    }
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

                ImGui::Text("ID: %s", id.c_str());

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