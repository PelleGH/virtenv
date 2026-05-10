#pragma once
#include <string>
#include <unordered_map>

enum class QuestStatus {
    NotStarted,
    Active,
    Completed
};

struct Quest {
    std::string id;
    std::string name;
    QuestStatus status = QuestStatus::NotStarted;

    std::string requiredEvent;
    std::string targetId;
    int requiredAmount = 1;
    int currentAmount = 0;

    std::string rewardItem;
};

class QuestManager {
public:
    void addQuest(const Quest& quest);
    void startQuest(const std::string& questId);
    void completeQuest(const std::string& questId);

    void onEvent(const std::string& eventType, const std::string& targetId);

    bool isQuestActive(const std::string& questId) const;
    bool isQuestCompleted(const std::string& questId) const;

private:
    std::unordered_map<std::string, Quest> quests;
};