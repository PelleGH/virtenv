#pragma once

#include <vector>
#include <unordered_set>
#include "../ecs/Components.h"

class ConditionManager
{
public:
    bool debugUnlocked = false;

    std::unordered_set<std::string> completedQuests;
    std::unordered_set<std::string> inventoryItems;
    std::unordered_set<std::string> solvedPuzzles;

    bool conditionsMet(const std::vector<Condition>& conditions)
    {
        for (const auto& condition : conditions)
        {
            if (!conditionMet(condition))
                return false;
        }

        return true;
    }

private:
    bool conditionMet(const Condition& condition)
    {
        if (condition.type == "Debug")
            return debugUnlocked;
        if (condition.type == "QuestCompleted")
        {
            return completedQuests.find(condition.id)
                != completedQuests.end();
        }

        if (condition.type == "HasItem")
        {
            return inventoryItems.find(condition.id)
                != inventoryItems.end();
        }

        if (condition.type == "PuzzleSolved")
        {
            return solvedPuzzles.find(condition.id)
                != solvedPuzzles.end();
        }
        return false;
    }
};