#include "SceneManager.h"

bool SceneManager::loadScene(const std::string& path)
{
    currentScene.unload();
    return currentScene.load(path);
}

// Queues a scene change to be applied at the end of the frame
void SceneManager::requestSceneChange(const std::string& path)
{
    pendingScenePath = path;
    hasPendingSceneChange = true;
}

// Called at the end of the frame to apply any pending scene changes
void SceneManager::applyPendingSceneChange()
{
    if (!hasPendingSceneChange)
        return;

    loadScene(pendingScenePath);

    pendingScenePath.clear();
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

