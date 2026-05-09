#pragma once

#include <string>
#include <nlohmann/json.hpp>

class DialogueManager
{
public:
    bool loadDialogue(const std::string& dialogueId);
    void startDialogue(const std::string& dialogueId);
    bool isActive() const { return active; }
    
    void update();
    void render();

private:
    bool active = false;
    std::string currentDialogueId;
    std::string currentNodeId;
    nlohmann::json dialogueData;
};