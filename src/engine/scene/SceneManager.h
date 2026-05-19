// SceneManager
// Controls active scene and handles safe scene switching.
// Scene changes are queued and applied at the end of the frame.

#pragma once

#include <string>
#include "Scene.h"
#include "raylib.h"

class SceneManager
{


public:
    void init(ResourceManager& rm);

    bool loadScene(const std::string& path, const std::string& spawnId = "default");

    void update(float dt);
    void shutdown();

    Scene& getCurrentScene();
    void requestSceneChange(const std::string& path, const std::string& spawnId = "default");
    void applyPendingSceneChange();

    void requestPlayerRespawn(); 

    void setProjectPath(const std::string& path) { projectBasePath = path; }
    std::string getProjectPath() const { return projectBasePath; }
private:
    Scene currentScene;
    std::string pendingScenePath;
    std::string pendingSpawnId = "default";
    bool hasPendingSceneChange = false;

    ResourceManager* resourceManager = nullptr;
    bool resetPlayerOnNextLoad = false;

    std::string projectBasePath = "";

   
};