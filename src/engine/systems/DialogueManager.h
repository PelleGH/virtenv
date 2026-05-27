#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "../managers/QuestManager.h"

class DialogueManager
{
public:
    void setQuestManager(QuestManager* manager);

    bool loadDialogue(const std::string& dialogueId);
    void startDialogue(const std::string& dialogueId);
    bool isActive() const { return active; }
    void setAssetRoot(const std::string& root);
    void update();
    void render();

private:
    void selectChoice(int choiceIndex);
    bool shouldShowChoice(const nlohmann::json& choice) const;
    std::vector<int> getVisibleChoiceIndices() const;
    
    bool active = false;
    std::string currentDialogueId;
    std::string currentNodeId;
    nlohmann::json dialogueData;
    std::string assetRoot = "";
    QuestManager* questManager = nullptr;
};