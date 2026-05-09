#include "DialogueManager.h"
#include "raylib.h"
#include <fstream>
#include <iostream>

bool DialogueManager::loadDialogue(const std::string& dialogueSetId)
{
    std::string path = "assets/dialogue/" + dialogueSetId + ".json";

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

    auto& choices = node["choices"];

    if (IsKeyPressed(KEY_ONE))
    {
        if (choices.size() > 0 && choices[0].is_object())
        {
            currentNodeId = choices[0].value("next", "end");

            if (currentNodeId == "end")
                active = false;
        }
    }

    if (IsKeyPressed(KEY_TWO))
    {
        if (choices.size() > 1 && choices[1].is_object())
        {
            currentNodeId = choices[1].value("next", "end");

            if (currentNodeId == "end")
                active = false;
        }
    }
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

        for (int i = 0; i < node["choices"].size(); i++)
        {
            if (!node["choices"][i].is_object())
                continue;

            std::string choiceText =
                std::to_string(i + 1) + ". " +
                node["choices"][i].value("text", "");

            DrawText(choiceText.c_str(), 110, y, 20, DARKGRAY);
            y += 28;
        }
    }
}