#pragma once

#include <string>
#include <unordered_map>
#include "../systems/ConditionManager.h"

enum class QuestStatus
{
    NotStarted,
    Active,
    Completed
};

struct Quest
{
    std::string id;
    std::string name;
    QuestStatus status = QuestStatus::NotStarted;

    std::string requiredEvent;
    std::string targetId;
    int requiredAmount = 1;
    int currentAmount = 0;

    std::string rewardItem;
};

class QuestManager
{
public:
    bool loadQuests(const std::string& path);

    void startQuest(const std::string& questId);
    void onEvent(const std::string& eventType, const std::string& targetId, int amount = 1);
    void completeQuest(const std::string& questId);

    bool isQuestActive(const std::string& questId) const;
    bool isQuestCompleted(const std::string& questId) const;
    void setRewardCallback(std::function<void(const std::string&)> callback);
    void setConditionManager(ConditionManager* manager);
    
    QuestStatus getQuestStatus(const std::string& questId) const;
private:
    std::unordered_map<std::string, Quest> quests;
    ConditionManager* conditionManager = nullptr;
    std::function<void(const std::string&)> rewardCallback;
};