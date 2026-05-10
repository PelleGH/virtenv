#include "QuestManager.h"
#include <iostream>

void QuestManager::addQuest(const Quest& quest) {
    quests[quest.id] = quest;
}

void QuestManager::startQuest(const std::string& questId) {
    auto it = quests.find(questId);
    if (it == quests.end()) return;

    if (it->second.status == QuestStatus::NotStarted) {
        it->second.status = QuestStatus::Active;
        std::cout << "Started quest: " << it->second.name << '\n';
    }
}

void QuestManager::completeQuest(const std::string& questId) {
    auto it = quests.find(questId);
    if (it == quests.end()) return;

    if (it->second.status == QuestStatus::Active) {
        it->second.status = QuestStatus::Completed;
        std::cout << "Completed quest: " << it->second.name << '\n';

        if (!it->second.rewardItem.empty()) {
            std::cout << "Reward: " << it->second.rewardItem << '\n';
        }
    }
}

void QuestManager::onEvent(const std::string& eventType, const std::string& targetId) {
    for (auto& [id, quest] : quests) {
        if (quest.status != QuestStatus::Active) continue;
        if (quest.requiredEvent != eventType) continue;
        if (quest.targetId != targetId) continue;

        quest.currentAmount++;

        if (quest.currentAmount >= quest.requiredAmount) {
            completeQuest(id);
        }
    }
}

bool QuestManager::isQuestActive(const std::string& questId) const {
    auto it = quests.find(questId);
    return it != quests.end() && it->second.status == QuestStatus::Active;
}

bool QuestManager::isQuestCompleted(const std::string& questId) const {
    auto it = quests.find(questId);
    return it != quests.end() && it->second.status == QuestStatus::Completed;
}