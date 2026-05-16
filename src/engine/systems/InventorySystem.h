#pragma once
#include <string>
#include "../messaging/Event.h"

class Scene;
class EventBus;
class ResourceManager;

class InventorySystem {
public:
    void initialize(Scene& scene, EventBus& eventBus, ResourceManager& resManager);

private:
    Scene* m_scene = nullptr;
    EventBus* m_eventBus = nullptr;
    ResourceManager* m_resManager = nullptr;

    void onAction(const ActionEvent& event);
    void onEquip(const EquipEvent& event);
    void onDropItem(const DropItemEvent& event);
};