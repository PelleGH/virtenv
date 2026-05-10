#include "SceneManager.h"

bool SceneManager::loadScene(const std::string& path, const std::string& spawnId)
{
    currentScene.unload();

    if (!currentScene.load(path))
        return false;

    currentScene.spawnPlayerAt(spawnId);

    return true;
}

// Queues a scene change to be applied at the end of the frame
void SceneManager::requestSceneChange(const std::string& path, const std::string& spawnId)
{
    pendingScenePath = path;
    pendingSpawnId = spawnId;
    hasPendingSceneChange = true;
}

// Called at the end of the frame to apply any pending scene changes
void SceneManager::applyPendingSceneChange()
{
    if (!hasPendingSceneChange)
        return;

    loadScene(pendingScenePath, pendingSpawnId);

    pendingScenePath.clear();
    pendingSpawnId = "default";
    hasPendingSceneChange = false;
}
void SceneManager::update(float dt)
{
    currentScene.update(dt);
}

void SceneManager::shutdown()
{
    currentScene.unload();
}

Scene& SceneManager::getCurrentScene()
{
    return currentScene;
}

