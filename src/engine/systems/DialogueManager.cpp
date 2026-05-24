#include "DialogueManager.h"
#include "raylib.h"
#include <fstream>
#include <iostream>
void DialogueManager::setAssetRoot(const std::string& root)
{
    assetRoot = root;

    if (!assetRoot.empty() && assetRoot.back() != '/' && assetRoot.back() != '\\')
    {
        assetRoot += "/";
    }
}
void DialogueManager::setQuestManager(QuestManager* manager)
{
    questManager = manager;
}
bool DialogueManager::loadDialogue(const std::string& dialogueSetId)
{
    std::string path = assetRoot + "dialogue/" + dialogueSetId + ".json";

    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cout << "Could not open dialogue file: " << path << '\n';
        return false;
    }

    file >> dialogueData;
    currentDialogueId = dialogueSetId;
    return true;
}

void DialogueManager::startDialogue(const std::string& dialogueSetId)
{
    if (!loadDialogue(dialogueSetId))
        return;

    active = true;
    currentNodeId = dialogueData.value("defaultStartNode", "intro");
}

void DialogueManager::update()
{
    if (!active)
        return;

    if (currentNodeId == "end")
    {
        active = false;
        return;
    }

    if (dialogueData.find("nodes") == dialogueData.end())
    {
        active = false;
        return;
    }

    auto& nodes = dialogueData["nodes"];

    if (nodes.find(currentNodeId) == nodes.end())
    {
        active = false;
        return;
    }

    auto& node = nodes[currentNodeId];

    if (node.find("choices") == node.end() || !node["choices"].is_array())
        return;

    if (IsKeyPressed(KEY_ONE))
        selectChoice(0);

    if (IsKeyPressed(KEY_TWO))
        selectChoice(1);

    if (IsKeyPressed(KEY_THREE))
        selectChoice(2);

    if (IsKeyPressed(KEY_FOUR))
        selectChoice(3);
}
void DialogueManager::selectChoice(int visibleChoiceIndex)
{
    std::vector<int> visibleChoices = getVisibleChoiceIndices();

    if (visibleChoiceIndex < 0 || visibleChoiceIndex >= visibleChoices.size())
        return;

    int realChoiceIndex = visibleChoices[visibleChoiceIndex];

    auto& choice = dialogueData["nodes"][currentNodeId]["choices"][realChoiceIndex];

    std::string startQuest = choice.value("startQuest", "");

    if (!startQuest.empty() && questManager != nullptr)
    {
        questManager->startQuest(startQuest);
    }

    currentNodeId = choice.value("next", "end");

    if (currentNodeId == "end")
        active = false;
}
void DialogueManager::render()
{
    if (!active)
        return;

    if (currentNodeId == "end")
    {
        active = false;
        return;
    }

    if (dialogueData.find("nodes") == dialogueData.end())
        return;

    const auto& nodes = dialogueData["nodes"];

    if (nodes.find(currentNodeId) == nodes.end())
        return;

    const auto& node = nodes[currentNodeId];

    if (!node.is_object())
        return;

    DrawRectangle(80, 500, 1120, 170, LIGHTGRAY);
    DrawRectangleLines(80, 500, 1120, 170, BLACK);

    std::string text = node.value("text", "");
    DrawText(text.c_str(), 110, 525, 24, BLACK);

    if (node.find("choices") != node.end() && node["choices"].is_array())
    {
        int y = 570;

    std::vector<int> visibleChoices = getVisibleChoiceIndices();

    for (int displayIndex = 0; displayIndex < visibleChoices.size(); displayIndex++)
    {
        int realChoiceIndex = visibleChoices[displayIndex];
        const auto& choice = node["choices"][realChoiceIndex];

        std::string choiceText =
            std::to_string(displayIndex + 1) + ". " +
            choice.value("text", "");

        Color choiceColor = DARKGRAY;

        if (!choice.value("startQuest", "").empty())
        {
            choiceColor = ORANGE;
        }

        DrawText(choiceText.c_str(), 110, y, 20, choiceColor);
        y += 28;
    }
    }
}

bool DialogueManager::shouldShowChoice(const nlohmann::json& choice) const
{
    if (!choice.contains("showIfQuestStatus"))
        return true;

    if (questManager == nullptr)
        return false;

    auto condition = choice["showIfQuestStatus"];

    std::string questId = condition.value("questId", "");
    std::string status = condition.value("status", "");

    QuestStatus currentStatus = questManager->getQuestStatus(questId);

    if (status == "NotStarted")
        return currentStatus == QuestStatus::NotStarted;

    if (status == "Active")
        return currentStatus == QuestStatus::Active;

    if (status == "Completed")
        return currentStatus == QuestStatus::Completed;

    return false;
}
std::vector<int> DialogueManager::getVisibleChoiceIndices() const
{
    std::vector<int> visibleChoices;

    if (!dialogueData.contains("nodes"))
        return visibleChoices;

    const auto& nodes = dialogueData["nodes"];

    if (!nodes.contains(currentNodeId))
        return visibleChoices;

    const auto& node = nodes[currentNodeId];

    if (!node.contains("choices") || !node["choices"].is_array())
        return visibleChoices;

    const auto& choices = node["choices"];

    for (int i = 0; i < choices.size(); i++)
    {
        if (!choices[i].is_object())
            continue;

        if (shouldShowChoice(choices[i]))
        {
            visibleChoices.push_back(i);
        }
    }

    return visibleChoices;
}