#include "QuestManager.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

QuestStatus QuestManager::getQuestStatus(const std::string& questId) const
{
    auto it = quests.find(questId);

    if (it == quests.end())
        return QuestStatus::NotStarted;

    return it->second.status;
}

bool QuestManager::loadQuests(const std::string& path)
{
    std::ifstream file(path);

    if (!file.is_open())
    {
        std::cout << "Could not open quests file: " << path << '\n';
        return false;
    }

    nlohmann::json data;
    file >> data;

    if (!data.contains("quests") || !data["quests"].is_array())
    {
        std::cout << "Invalid quests file: missing quests array\n";
        return false;
    }

    quests.clear();

    for (const auto& questJson : data["quests"])
    {
        Quest quest;
        quest.id = questJson.value("id", "");
        quest.name = questJson.value("name", quest.id);
        quest.requiredEvent = questJson.value("requiredEvent", "");
        quest.targetId = questJson.value("targetId", "");
        quest.requiredAmount = questJson.value("requiredAmount", 1);
        quest.currentAmount = 0;
        quest.rewardItem = questJson.value("rewardItem", "");

        if (quest.id.empty())
            continue;

        quests[quest.id] = quest;
    }

    std::cout << "Loaded " << quests.size() << " quests\n";
    return true;
}

void QuestManager::startQuest(const std::string& questId)
{
    auto it = quests.find(questId);

    if (it == quests.end())
    {
        std::cout << "Quest not found: " << questId << '\n';
        return;
    }

    Quest& quest = it->second;

    if (quest.status != QuestStatus::NotStarted)
        return;

    quest.status = QuestStatus::Active;
    quest.currentAmount = 0;

    std::cout << "Quest started: " << quest.name << '\n';
}

void QuestManager::onEvent(const std::string& eventType, const std::string& targetId)
{
    for (auto& pair : quests)
    {
        Quest& quest = pair.second;

        if (quest.status != QuestStatus::Active)
            continue;

        if (quest.requiredEvent != eventType)
            continue;

        if (quest.targetId != targetId)
            continue;

        quest.currentAmount++;

        std::cout << "Quest progress: " << quest.name << " "
                  << quest.currentAmount << "/"
                  << quest.requiredAmount << '\n';

        if (quest.currentAmount >= quest.requiredAmount)
        {
            completeQuest(quest.id);
        }
    }
}

void QuestManager::completeQuest(const std::string& questId)
{
    auto it = quests.find(questId);

    if (it == quests.end())
        return;

    Quest& quest = it->second;

    if (quest.status == QuestStatus::Completed)
        return;

    quest.status = QuestStatus::Completed;

    if (conditionManager != nullptr)
    {
        conditionManager->completedQuests.insert(quest.id);
    }

    std::cout << "Quest completed: " << quest.name << '\n';

    if (!quest.rewardItem.empty())
    {
        std::cout << "Reward: " << quest.rewardItem << '\n';
    }
}

bool QuestManager::isQuestActive(const std::string& questId) const
{
    auto it = quests.find(questId);
    return it != quests.end() && it->second.status == QuestStatus::Active;
}

bool QuestManager::isQuestCompleted(const std::string& questId) const
{
    auto it = quests.find(questId);
    return it != quests.end() && it->second.status == QuestStatus::Completed;
}

void QuestManager::setConditionManager(ConditionManager* manager)
{
    conditionManager = manager;
}