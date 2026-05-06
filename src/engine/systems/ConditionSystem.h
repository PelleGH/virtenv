#pragma once

class Scene;
class ConditionManager;

class ConditionSystem
{
public:
    void update(Scene& scene, ConditionManager& conditionManager);
};