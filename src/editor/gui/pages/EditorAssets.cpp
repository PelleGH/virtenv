#include "../EditorPanels.h"
#include "imgui.h"

#include <fstream>
#include <iostream>
#include <filesystem>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;
static bool SaveJsonFile(const std::string& path, const json& data)
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
static std::string GetDialogueFolder(const EditorContext& context)
{
    return context.projectPath + "assets/dialogue/";
}

static void AddNewDialogue(EditorContext& context)
{
    fs::create_directories(GetDialogueFolder(context));

    std::string id = "new_dialogue";
    std::string path = GetDialogueFolder(context) + id + ".json";

    int counter = 1;
    while (fs::exists(path))
    {
        id = "new_dialogue_" + std::to_string(counter);
        path = GetDialogueFolder(context) + id + ".json";
        counter++;
    }

    json data = {
        {"defaultStartNode", "intro"},
        {"nodes", {
            {
                "intro",
                {
                    {"text", "Hello."},
                    {"choices", json::array({
                        {
                            {"text", "Goodbye."},
                            {"next", "end"}
                        }
                    })}
                }
            }
        }}
    };

    if (SaveJsonFile(path, data))
    {
        context.selection.type = SelectionType::AssetDialogue;
        context.selection.assetId = id;
        context.dirty = true;
        context.buildOutdated = true;
    }
}

static std::string GetItemsPath(const EditorContext& context)
{
    return context.projectPath + "assets/items.json";
}

static std::string GetQuestsPath(const EditorContext& context)
{
    return context.projectPath + "assets/quests.json";
}

static json LoadJsonFile(const std::string& path, const std::string& rootArrayName)
{
    std::ifstream file(path);

    if (!file.is_open())
    {
        json empty;
        empty[rootArrayName] = json::array();
        return empty;
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



static bool IdExists(const json& array, const std::string& id)
{
    for (const auto& entry : array)
    {
        if (entry.value("id", "") == id)
            return true;
    }

    return false;
}

static std::string MakeUniqueId(const json& array, const std::string& baseId)
{
    if (!IdExists(array, baseId))
        return baseId;

    int counter = 1;

    while (true)
    {
        std::string candidate = baseId + "_" + std::to_string(counter);

        if (!IdExists(array, candidate))
            return candidate;

        counter++;
    }
}

static void AddNewItem(EditorContext& context)
{
    json data = LoadJsonFile(GetItemsPath(context), "items");

    std::string id = MakeUniqueId(data["items"], "new_item");

    data["items"].push_back({
        {"id", id},
        {"name", "New Item"},
        {"type", "Quest"},
        {"slot", "None"},
        {"damageBonus", 0},
        {"healthBonus", 0},
        {"defenseBonus", 0}
    });

    if (SaveJsonFile(GetItemsPath(context), data))
    {
        context.resourceManager.loadItems("items.json");
        context.selection.type = SelectionType::AssetItem;
        context.selection.assetId = id;
        context.dirty = true;
        context.buildOutdated = true;
    }
}

static void AddNewQuest(EditorContext& context)
{
    json data = LoadJsonFile(GetQuestsPath(context), "quests");

    std::string id = MakeUniqueId(data["quests"], "new_quest");

    data["quests"].push_back({
        {"id", id},
        {"name", "New Quest"},
        {"requiredEvent", "enemy_killed"},
        {"targetId", "slime"},
        {"requiredAmount", 1},
        {"rewardItem", ""}
    });

    if (SaveJsonFile(GetQuestsPath(context), data))
    {
        context.selection.type = SelectionType::AssetQuest;
        context.selection.assetId = id;
        context.dirty = true;
        context.buildOutdated = true;
    }
}
void DrawAssets(EditorContext& context)
{
    // In order to know which project specific assets window belong to
    std::string assetsTitle = "Assets";

    if (!context.projectName.empty()) {
        assetsTitle = assetsTitle + " (" + context.projectName + ")";
    }

    // ### acts like a hidden ID for ImGUi to remember that the assets window is the same,
    // even when the name changes
    assetsTitle = assetsTitle + "###AssetsWindow";

    ImGui::Begin(assetsTitle.c_str(), NULL, ImGuiWindowFlags_NoFocusOnAppearing);

    if (ImGui::TreeNode("Textures")) {

        // Loop through all textures in resource
        for (const auto& [id, tex] : context.resourceManager.GetAllTextures()) {

            // If current id equals editors marker / pointer
           bool isSelected = (context.selection.type == SelectionType::AssetTexture && context.selection.assetId == id);

           // Write the row and update editors context when clicked
           if (ImGui::Selectable(id.c_str(), isSelected)) {
                context.selection.type = SelectionType::AssetTexture;
                context.selection.assetId = id;
           }
        }
        ImGui::TreePop(); 
    }

    if (ImGui::TreeNode("Models")) {
        
        // Loop through all models in resource
        for (const auto& [id, model] : context.resourceManager.GetAllModels()) {

            // If current id equals editors marker / pointer
            bool isSelected = (context.selection.type == SelectionType::AssetModel && context.selection.assetId == id);
            
            // Write the row and update editors context when clicked
            if (ImGui::Selectable(id.c_str(), isSelected)) {
                context.selection.type = SelectionType::AssetModel;
                context.selection.assetId = id;
            }
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Items"))
    {
        if (ImGui::Button("+ New Item"))
        {
            AddNewItem(context);
        }

        ImGui::Separator();

        for (const auto& [id, item] : context.resourceManager.GetAllItems())
        {
            char label[128];
            snprintf(label, sizeof(label), "%s (%s)", id.c_str(), item.name.c_str());

            bool isSelected =
                context.selection.type == SelectionType::AssetItem &&
                context.selection.assetId == id;

            if (ImGui::Selectable(label, isSelected))
            {
                context.selection.type = SelectionType::AssetItem;
                context.selection.assetId = id;
            }
        }

        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Dialogue"))
    {
        if (ImGui::Button("+ New Dialogue"))
        {
            AddNewDialogue(context);
        }

        ImGui::Separator();

        std::string folder = GetDialogueFolder(context);

        if (fs::exists(folder))
        {
            for (const auto& entry : fs::directory_iterator(folder))
            {
                if (!entry.is_regular_file())
                    continue;

                if (entry.path().extension() != ".json")
                    continue;

                std::string id = entry.path().stem().string();

                bool isSelected =
                    context.selection.type == SelectionType::AssetDialogue &&
                    context.selection.assetId == id;

                if (ImGui::Selectable(id.c_str(), isSelected))
                {
                    context.selection.type = SelectionType::AssetDialogue;
                    context.selection.assetId = id;
                }
            }
        }

        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Quests"))
    {
        if (ImGui::Button("+ New Quest"))
        {
            AddNewQuest(context);
        }

        ImGui::Separator();

        json questData = LoadJsonFile(GetQuestsPath(context), "quests");

        for (const auto& quest : questData["quests"])
        {
            std::string id = quest.value("id", "");
            std::string name = quest.value("name", id);

            if (id.empty())
                continue;

            std::string label = id + " (" + name + ")";

            bool isSelected =
                context.selection.type == SelectionType::AssetQuest &&
                context.selection.assetId == id;

            if (ImGui::Selectable(label.c_str(), isSelected))
            {
                context.selection.type = SelectionType::AssetQuest;
                context.selection.assetId = id;
            }
        }

        ImGui::TreePop();
    }
    ImGui::End();
}