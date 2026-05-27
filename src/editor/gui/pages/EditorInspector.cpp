#include "../EditorPanels.h"
#include "imgui.h"
#include "../../../engine/scene/SceneLoader.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <cstring>
#include <vector>
#include <string>
#include <unordered_set>
#include <algorithm>

static std::string HiddenId(const std::string& label);
static void DrawFieldLabel(const std::string& label);
using json = nlohmann::json;

static void CopyStringToBuffer(char* buffer, size_t bufferSize, const std::string& value)
{
    snprintf(buffer, bufferSize, "%s", value.c_str());
}

static bool InputJsonString(json& object, const char* fieldName, const char* label, size_t bufferSize = 256)
{
    std::string current = object.value(fieldName, "");

    std::vector<char> buffer(bufferSize);
    CopyStringToBuffer(buffer.data(), buffer.size(), current);

    if (ImGui::InputText(label, buffer.data(), buffer.size()))
    {
        object[fieldName] = std::string(buffer.data());
        return true;
    }

    return false;
}

static bool InputPlainString(std::string& value, const char* label, size_t bufferSize = 256)
{
    std::vector<char> buffer(bufferSize);
    CopyStringToBuffer(buffer.data(), buffer.size(), value);

    if (ImGui::InputText(label, buffer.data(), buffer.size()))
    {
        value = std::string(buffer.data());
        return true;
    }

    return false;
}

static std::string MakeUniqueDialogueNodeId(const json& nodes, const std::string& baseId)
{
    if (!nodes.contains(baseId))
        return baseId;

    int counter = 1;

    while (true)
    {
        std::string candidate = baseId + "_" + std::to_string(counter);

        if (!nodes.contains(candidate))
            return candidate;

        counter++;
    }
}

static bool RenameDialogueNode(json& data, const std::string& oldId, const std::string& newId)
{
    if (oldId == newId)
        return false;

    if (newId.empty())
        return false;

    if (!data.contains("nodes") || !data["nodes"].is_object())
        return false;

    json& nodes = data["nodes"];

    if (!nodes.contains(oldId))
        return false;

    if (nodes.contains(newId))
        return false;

    nodes[newId] = nodes[oldId];
    nodes.erase(oldId);

    if (data.value("defaultStartNode", "") == oldId)
    {
        data["defaultStartNode"] = newId;
    }

    for (auto& [nodeId, node] : nodes.items())
    {
        if (!node.contains("choices") || !node["choices"].is_array())
            continue;

        for (auto& choice : node["choices"])
        {
            if (choice.value("next", "") == oldId)
            {
                choice["next"] = newId;
            }
        }
    }

    return true;
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
static json LoadEditorJsonFile(const std::string& path, const std::string& rootArrayName)
{
    std::ifstream file(path);

    if (!file.is_open())
    {
        json data;
        data[rootArrayName] = json::array();
        return data;
    }

    json data;

    try
    {
        file >> data;
    }
    catch (const json::exception& e)
    {
        std::cout << "[Editor] Failed to parse " << path << ": " << e.what() << '\n';
        data[rootArrayName] = json::array();
    }

    if (!data.contains(rootArrayName) || !data[rootArrayName].is_array())
    {
        data[rootArrayName] = json::array();
    }

    return data;
}

static bool SaveEditorJsonFile(const std::string& path, const json& data)
{
    std::ofstream file(path);

    if (!file.is_open())
    {
        std::cout << "[Editor] Failed to save json file: " << path << '\n';
        return false;
    }

    file << data.dump(4);
    return true;
}


static void DrawEditableItem(EditorContext& context)
{
    const std::string path = context.projectPath + "assets/items.json";
    json data = LoadEditorJsonFile(path, "items");

    for (auto& item : data["items"])
    {
        if (item.value("id", "") != context.selection.assetId)
            continue;

        char idBuf[128];
        char nameBuf[128];
        char slotBuf[64];
        char typeBuf[64];

        CopyStringToBuffer(typeBuf, sizeof(typeBuf), item.value("type", "Misc"));
        CopyStringToBuffer(idBuf, sizeof(idBuf), item.value("id", ""));
        CopyStringToBuffer(nameBuf, sizeof(nameBuf), item.value("name", ""));
        CopyStringToBuffer(slotBuf, sizeof(slotBuf), item.value("slot", "None"));

        int damageBonus = item.value("damageBonus", 0);
        int healthBonus = item.value("healthBonus", 0);
        int defenseBonus = item.value("defenseBonus", 0);

        ImGui::Text("Asset: Item Data");
        ImGui::Separator();

        bool changed = false;

        changed |= ImGui::InputText("ID", idBuf, sizeof(idBuf));
        changed |= ImGui::InputText("Name", nameBuf, sizeof(nameBuf));
        const char* itemTypes[] = { "Equipment", "Consumable", "Quest", "Misc" };
        int typeIndex = 3;

        std::string currentType = item.value("type", "Misc");

        for (int i = 0; i < 4; i++)
        {
            if (currentType == itemTypes[i])
            {
                typeIndex = i;
                break;
            }
        }

        if (ImGui::Combo("Type", &typeIndex, itemTypes, 4))
        {
            CopyStringToBuffer(typeBuf, sizeof(typeBuf), itemTypes[typeIndex]);
            changed = true;
        }
        const char* slots[] = { "None", "Weapon", "Armor", "Consumable" };
        int slotIndex = 0;
        std::string currentSlot = item.value("slot", "None");

        for (int i = 0; i < 4; i++)
        {
            if (currentSlot == slots[i])
            {
                slotIndex = i;
                break;
            }
        }

        if (ImGui::Combo("Slot", &slotIndex, slots, 4))
        {
            CopyStringToBuffer(slotBuf, sizeof(slotBuf), slots[slotIndex]);
            changed = true;
        }

        changed |= ImGui::InputInt("Damage Bonus", &damageBonus);
        changed |= ImGui::InputInt("Health Bonus", &healthBonus);
        changed |= ImGui::InputInt("Defense Bonus", &defenseBonus);
        ImGui::Separator();

        if (ImGui::Button("Delete Item"))
        {
            std::string deletedId = context.selection.assetId;

            for (auto it = data["items"].begin(); it != data["items"].end(); ++it)
            {
                if (it->value("id", "") == deletedId)
                {
                    data["items"].erase(it);
                    break;
                }
            }

            if (SaveEditorJsonFile(path, data))
            {
                context.resourceManager.loadItems("items.json");
                context.selection.type = SelectionType::None;
                context.selection.assetId.clear();
                context.dirty = true;
                context.buildOutdated = true;
            }

            return;
        }
        if (changed)
        {
            item["id"] = std::string(idBuf);
            item["name"] = std::string(nameBuf);
            item["type"] = std::string(typeBuf);
            item["slot"] = std::string(slotBuf);
            item["damageBonus"] = damageBonus;
            item["healthBonus"] = healthBonus;
            item["defenseBonus"] = defenseBonus;

            if (SaveEditorJsonFile(path, data))
            {
                context.selection.assetId = std::string(idBuf);
                context.resourceManager.loadItems("items.json");
                context.dirty = true;
                context.buildOutdated = true;
            }
        }

        ImGui::SeparatorText("Raw JSON");
        ImGui::BeginChild("ItemJsonBox", ImVec2(0, 150), true);
        ImGui::TextWrapped("%s", item.dump(2).c_str());
        ImGui::EndChild();

        return;
    }

    ImGui::Text("Selected item no longer exists.");
}
static std::vector<std::string> GetProjectItemIds(const EditorContext& context)
{
    std::vector<std::string> ids;

    for (const auto& [id, item] : context.resourceManager.GetAllItems())
        ids.push_back(id);

    std::sort(ids.begin(), ids.end());
    return ids;
}
static void DrawEditableQuest(EditorContext& context)
{
    const std::string path = context.projectPath + "assets/quests.json";
    json data = LoadEditorJsonFile(path, "quests");

    for (auto& quest : data["quests"])
    {
        if (quest.value("id", "") != context.selection.assetId)
            continue;

        char idBuf[128];
        char nameBuf[128];
        char eventBuf[128];
        char targetBuf[128];
        char rewardBuf[128];

        CopyStringToBuffer(idBuf, sizeof(idBuf), quest.value("id", ""));
        CopyStringToBuffer(nameBuf, sizeof(nameBuf), quest.value("name", ""));
        CopyStringToBuffer(eventBuf, sizeof(eventBuf), quest.value("requiredEvent", "enemy_killed"));
        CopyStringToBuffer(targetBuf, sizeof(targetBuf), quest.value("targetId", ""));
        CopyStringToBuffer(rewardBuf, sizeof(rewardBuf), quest.value("rewardItem", ""));

        int requiredAmount = quest.value("requiredAmount", 1);

        ImGui::Text("Asset: Quest Data");
        ImGui::Separator();

        bool changed = false;

        changed |= ImGui::InputText("ID", idBuf, sizeof(idBuf));
        changed |= ImGui::InputText("Name", nameBuf, sizeof(nameBuf));

        const char* eventTypes[] = {
            "enemy_killed",
            "quest_completed",
            "item_collected",
            "dialogue_finished"
        };

        int eventIndex = 0;
        std::string currentEvent = quest.value("requiredEvent", "enemy_killed");

        for (int i = 0; i < 4; i++)
        {
            if (currentEvent == eventTypes[i])
            {
                eventIndex = i;
                break;
            }
        }

        if (ImGui::Combo("Required Event", &eventIndex, eventTypes, 4))
        {
            CopyStringToBuffer(eventBuf, sizeof(eventBuf), eventTypes[eventIndex]);
            changed = true;
        }

        changed |= ImGui::InputText("Target ID", targetBuf, sizeof(targetBuf));
        changed |= ImGui::InputInt("Required Amount", &requiredAmount);
        std::string rewardItem = quest.value("rewardItem", "");
        std::vector<std::string> itemIds = GetProjectItemIds(context);

        if (DrawStringDropdownWithNone("Reward Item", rewardItem, itemIds))
        {
            CopyStringToBuffer(rewardBuf, sizeof(rewardBuf), rewardItem);
            changed = true;
        }

        if (requiredAmount < 1)
            requiredAmount = 1;

        if (changed)
        {
            quest["id"] = std::string(idBuf);
            quest["name"] = std::string(nameBuf);
            quest["requiredEvent"] = std::string(eventBuf);
            quest["targetId"] = std::string(targetBuf);
            quest["requiredAmount"] = requiredAmount;
            quest["rewardItem"] = std::string(rewardBuf);

            if (SaveEditorJsonFile(path, data))
            {
                context.selection.assetId = std::string(idBuf);
                context.dirty = true;
                context.buildOutdated = true;
            }
        }
        ImGui::Separator();

        if (ImGui::Button("Delete Quest"))
        {
            std::string deletedId = context.selection.assetId;

            for (auto it = data["quests"].begin(); it != data["quests"].end(); ++it)
            {
                if (it->value("id", "") == deletedId)
                {
                    data["quests"].erase(it);
                    break;
                }
            }

            if (SaveEditorJsonFile(path, data))
            {
                context.selection.type = SelectionType::None;
                context.selection.assetId.clear();
                context.dirty = true;
                context.buildOutdated = true;
            }

            return;
        }
        ImGui::SeparatorText("Raw JSON");
        ImGui::BeginChild("QuestJsonBox", ImVec2(0, 150), true);
        ImGui::TextWrapped("%s", quest.dump(2).c_str());
        ImGui::EndChild();

        return;
    }

    ImGui::Text("Selected quest no longer exists.");
}
static void DrawEditableDialogue(EditorContext& context)
{
    std::string path =
        context.projectPath + "assets/dialogue/" + context.selection.assetId + ".json";

    json data;

    {
        std::ifstream file(path);

        if (file.is_open())
        {
            try
            {
                file >> data;
            }
            catch (const json::exception& e)
            {
                ImGui::Text("Failed to parse dialogue JSON: %s", e.what());
                return;
            }
        }
        else
        {
            ImGui::Text("Could not open dialogue file.");
            return;
        }
    }

    if (!data.contains("defaultStartNode") || !data["defaultStartNode"].is_string())
    {
        data["defaultStartNode"] = "intro";
    }

    if (!data.contains("nodes") || !data["nodes"].is_object())
    {
        data["nodes"] = json::object();
    }

    if (data["nodes"].empty())
    {
        data["nodes"]["intro"] = {
            {"text", "Hello."},
            {"choices", json::array({
                {
                    {"text", "Goodbye."},
                    {"next", "end"}
                }
            })}
        };
        data["defaultStartNode"] = "intro";
        SaveEditorJsonFile(path, data);
    }

    bool changed = false;

    ImGui::Text("Asset: Dialogue Data");
    ImGui::Separator();

    ImGui::Text("Dialogue ID: %s", context.selection.assetId.c_str());

    std::string defaultStartNode = data.value("defaultStartNode", "intro");
    if (InputPlainString(defaultStartNode, "Default Start Node"))
    {
        data["defaultStartNode"] = defaultStartNode;
        changed = true;
    }

    ImGui::Separator();

    if (ImGui::Button("+ Add Node"))
    {
        std::string newNodeId = MakeUniqueDialogueNodeId(data["nodes"], "new_node");

        data["nodes"][newNodeId] = {
            {"text", "New dialogue text."},
            {"choices", json::array({
                {
                    {"text", "Continue."},
                    {"next", "end"}
                }
            })}
        };

        changed = true;
    }

    ImGui::SameLine();

    if (ImGui::Button("Delete Dialogue"))
    {
        std::filesystem::remove(path);

        context.selection.type = SelectionType::None;
        context.selection.assetId.clear();
        context.dirty = true;
        context.buildOutdated = true;

        return;
    }

    ImGui::SeparatorText("Nodes");

    std::vector<std::string> nodeIds;

    for (auto& [nodeId, node] : data["nodes"].items())
    {
        nodeIds.push_back(nodeId);
    }

    for (const std::string& nodeId : nodeIds)
    {
        if (!data["nodes"].contains(nodeId))
            continue;

        json& node = data["nodes"][nodeId];

        if (!node.contains("choices") || !node["choices"].is_array())
        {
            node["choices"] = json::array();
            changed = true;
        }

        std::string header = nodeId;

        if (nodeId == data.value("defaultStartNode", ""))
        {
            header += "  [START]";
        }

        if (ImGui::TreeNode(header.c_str()))
        {
            std::string editedNodeId = nodeId;
            std::string nodeIdLabel = "Node ID###node_id_" + nodeId;

            if (InputPlainString(editedNodeId, nodeIdLabel.c_str()))
            {
                if (RenameDialogueNode(data, nodeId, editedNodeId))
                {
                    changed = true;
                    ImGui::TreePop();
                    break;
                }
            }

            std::string makeStartLabel = "Set As Start###set_start_" + nodeId;
            if (ImGui::Button(makeStartLabel.c_str()))
            {
                data["defaultStartNode"] = nodeId;
                changed = true;
            }

            ImGui::SameLine();

            std::string deleteNodeLabel = "Delete Node###delete_node_" + nodeId;
            if (ImGui::Button(deleteNodeLabel.c_str()))
            {
                data["nodes"].erase(nodeId);

                if (data.value("defaultStartNode", "") == nodeId)
                {
                    if (!data["nodes"].empty())
                    {
                        data["defaultStartNode"] = data["nodes"].begin().key();
                    }
                    else
                    {
                        data["defaultStartNode"] = "intro";
                    }
                }

                for (auto& [otherNodeId, otherNode] : data["nodes"].items())
                {
                    if (!otherNode.contains("choices") || !otherNode["choices"].is_array())
                        continue;

                    for (auto& choice : otherNode["choices"])
                    {
                        if (choice.value("next", "") == nodeId)
                        {
                            choice["next"] = "end";
                        }
                    }
                }

                changed = true;
                ImGui::TreePop();
                break;
            }

            ImGui::Separator();

            std::string textLabel = "Text###text_" + nodeId;
            changed |= InputJsonString(node, "text", textLabel.c_str(), 512);

            ImGui::SeparatorText("Choices");

            json& choices = node["choices"];

            for (int i = 0; i < static_cast<int>(choices.size()); i++)
            {
                json& choice = choices[i];

                std::string choiceHeader =
                    "Choice " + std::to_string(i + 1) + "###choice_" + nodeId + "_" + std::to_string(i);

                if (ImGui::TreeNode(choiceHeader.c_str()))
                {
                    std::string choiceTextLabel =
                        "Choice Text###choice_text_" + nodeId + "_" + std::to_string(i);

                    changed |= InputJsonString(choice, "text", choiceTextLabel.c_str(), 256);

                    std::string currentNext = choice.value("next", "end");
                    std::vector<std::string> nextOptions = nodeIds;
                    nextOptions.push_back("end");

                    std::string nextLabel =
                        "Next Node###next_node_" + nodeId + "_" + std::to_string(i);

                    if (DrawStringDropdownWithNone(nextLabel.c_str(), currentNext, nextOptions))
                    {
                        if (currentNext.empty())
                            currentNext = "end";

                        choice["next"] = currentNext;
                        changed = true;
                    }

                    std::string startQuest = choice.value("startQuest", "");
                    std::vector<std::string> questIds;

                    const std::string questsPath = context.projectPath + "assets/quests.json";
                    json questData = LoadEditorJsonFile(questsPath, "quests");

                    for (const auto& quest : questData["quests"])
                    {
                        std::string questId = quest.value("id", "");
                        if (!questId.empty())
                            questIds.push_back(questId);
                    }

                    std::sort(questIds.begin(), questIds.end());

                    std::string startQuestLabel =
                        "Start Quest###start_quest_" + nodeId + "_" + std::to_string(i);

                    if (DrawStringDropdownWithNone(startQuestLabel.c_str(), startQuest, questIds))
                    {
                        if (startQuest.empty())
                            choice.erase("startQuest");
                        else
                            choice["startQuest"] = startQuest;

                        changed = true;
                    }

                    bool hasQuestCondition = choice.contains("showIfQuestStatus");

                    std::string checkboxLabel =
                        "Show Only If Quest Status###quest_cond_" + nodeId + "_" + std::to_string(i);

                    if (ImGui::Checkbox(checkboxLabel.c_str(), &hasQuestCondition))
                    {
                        if (hasQuestCondition)
                        {
                            choice["showIfQuestStatus"] = {
                                {"questId", ""},
                                {"status", "NotStarted"}
                            };
                        }
                        else
                        {
                            choice.erase("showIfQuestStatus");
                        }

                        changed = true;
                    }

                    if (hasQuestCondition)
                    {
                        json& condition = choice["showIfQuestStatus"];

                        std::string conditionQuestId = condition.value("questId", "");
                        std::string questConditionLabel =
                            "Quest###quest_condition_id_" + nodeId + "_" + std::to_string(i);

                        if (DrawStringDropdownWithNone(questConditionLabel.c_str(), conditionQuestId, questIds))
                        {
                            condition["questId"] = conditionQuestId;
                            changed = true;
                        }

                        const char* statuses[] = { "NotStarted", "Active", "Completed" };
                        int statusIndex = 0;

                        std::string currentStatus = condition.value("status", "NotStarted");

                        for (int s = 0; s < 3; s++)
                        {
                            if (currentStatus == statuses[s])
                            {
                                statusIndex = s;
                                break;
                            }
                        }

                        std::string statusLabel =
                            "Status###quest_condition_status_" + nodeId + "_" + std::to_string(i);

                        if (ImGui::Combo(statusLabel.c_str(), &statusIndex, statuses, 3))
                        {
                            condition["status"] = statuses[statusIndex];
                            changed = true;
                        }
                    }

                    std::string deleteChoiceLabel =
                        "Delete Choice###delete_choice_" + nodeId + "_" + std::to_string(i);

                    if (ImGui::Button(deleteChoiceLabel.c_str()))
                    {
                        choices.erase(choices.begin() + i);
                        changed = true;
                        ImGui::TreePop();
                        break;
                    }

                    ImGui::TreePop();
                }
            }

            std::string addChoiceLabel = "+ Add Choice###add_choice_" + nodeId;

            if (ImGui::Button(addChoiceLabel.c_str()))
            {
                choices.push_back({
                    {"text", "New choice."},
                    {"next", "end"}
                });

                changed = true;
            }

            ImGui::TreePop();
        }
    }

    if (changed)
    {
        if (SaveEditorJsonFile(path, data))
        {
            context.dirty = true;
            context.buildOutdated = true;
        }
    }

    ImGui::SeparatorText("Raw JSON");

    ImGui::BeginChild("DialogueJsonBox", ImVec2(0, 220), true);
    ImGui::TextWrapped("%s", data.dump(2).c_str());
    ImGui::EndChild();
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
static std::vector<std::string> GetProjectModelIds(const EditorContext& context)
{
    std::vector<std::string> ids;

    for (const auto& [id, model] : context.resourceManager.GetAllModels())
        ids.push_back(id);

    std::sort(ids.begin(), ids.end());
    return ids;
}

static std::vector<std::string> GetProjectTextureIds(const EditorContext& context)
{
    std::vector<std::string> ids;

    for (const auto& [id, texture] : context.resourceManager.GetAllTextures())
        ids.push_back(id);

    std::sort(ids.begin(), ids.end());
    return ids;
}



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
        if (componentName == "Renderer" && fieldName == "modelID" && visibleData[fieldName].is_string())
        {
            std::string modelId = visibleData[fieldName].get<std::string>();

            if (DrawStringDropdownWithNone("modelID", modelId, GetProjectModelIds(context)))
            {
                visibleData[fieldName] = modelId;
                changed = true;
            }

            drawnFields.insert(fieldName);
            ImGui::PopID();
            ImGui::Spacing();
            continue;
        }

        if (componentName == "Renderer" && fieldName == "textureID" && visibleData[fieldName].is_string())
        {
            std::string textureId = visibleData[fieldName].get<std::string>();

            if (DrawStringDropdownWithNone("textureID", textureId, GetProjectTextureIds(context)))
            {
                visibleData[fieldName] = textureId;
                changed = true;
            }

            drawnFields.insert(fieldName);
            ImGui::PopID();
            ImGui::Spacing();
            continue;
        }
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
        if (componentName == "Renderer" && fieldName == "modelID" && fieldValue.is_string())
        {
            std::string modelId = fieldValue.get<std::string>();

            if (DrawStringDropdownWithNone("modelID", modelId, GetProjectModelIds(context)))
            {
                fieldValue = modelId;
                changed = true;
            }

            ImGui::PopID();
            ImGui::Spacing();
            continue;
        }

        if (componentName == "Renderer" && fieldName == "textureID" && fieldValue.is_string())
        {
            std::string textureId = fieldValue.get<std::string>();

            if (DrawStringDropdownWithNone("textureID", textureId, GetProjectTextureIds(context)))
            {
                fieldValue = textureId;
                changed = true;
            }

            ImGui::PopID();
            ImGui::Spacing();
            continue;
        }
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

        std::string currentTextureId = FindTextureIdForCubeModel(context, cube.modelID);
        std::vector<std::string> textureIds = GetProjectTextureIds(context);

        if (DrawStringDropdownWithNone("Texture ID", currentTextureId, textureIds))
        {
            std::string matchingModelId = FindCubeModelIdForTexture(context, currentTextureId);

            if (!matchingModelId.empty())
            {
                cube.modelID = matchingModelId;
                MarkSceneChanged(context);
            }
            else
            {
                std::cout << "[Inspector] No cube model found for texture: " << currentTextureId << "\n";
            }
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
    if (context.selection.type == SelectionType::AssetTexture)
    {
        ImGui::Text("Asset: Texture2D");
        ImGui::Separator();
        
        // Get texture from resource with saved id
        std::string id = context.selection.assetId;
        Texture2D tex = context.resourceManager.GetTexture(id);
        
        ImGui::Text("ID: %s", id.c_str());

        ImGui::SeparatorText("Properties");
        ImGui::Text("Resolution: %d x %d px", tex.width, tex.height);
    }

    if (context.selection.type == SelectionType::AssetModel)
    {
        ImGui::Text("Asset: 3D Model");
        ImGui::Separator();
        
        // Get model from resource with saved id
        std::string id = context.selection.assetId;
        Model model = context.resourceManager.GetModel(id);
        
        ImGui::Text("ID: %s", id.c_str());

        ImGui::SeparatorText("Properties");
        ImGui::Text("Meshes: %d", model.meshCount);
        ImGui::Text("Materials: %d", model.materialCount);
    }

    if (context.selection.type == SelectionType::AssetItem)
    {
        DrawEditableItem(context);
    }

    if (context.selection.type == SelectionType::AssetQuest)
    {
        DrawEditableQuest(context);
    }
    if (context.selection.type == SelectionType::AssetDialogue)
    {
        DrawEditableDialogue(context);
    }
    ImGui::End();
}
