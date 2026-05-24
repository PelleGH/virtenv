#include "ProjectManager.h"

#include "engine/scene/SceneLoader.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

void LoadProject(EditorContext& context, const std::string& chosenProjectFolder) {
    std::cout << "[Editor] Loading Project..." << std::endl;

    // Clear memory from old models and textures
    context.resourceManager.clear();

    // Update the search way to project
    context.resourceManager.SetProjectPath(chosenProjectFolder);

    // Load the new projects assets json
    context.resourceManager.LoadFromManifest("assets.json");

    std::string projectFolder = "Projects/" + chosenProjectFolder + "/";
    std::string settingsPath = projectFolder + "project.json";

    std::ifstream file(settingsPath);
    if (!file.is_open()) {
        std::cout << "[Editor] ERROR: Could not find project.json inside " << projectFolder << "!" << std::endl;
        return;
    }

    json projectData;
    file >> projectData;

    context.projectPath = projectFolder;
    context.projectName = projectData.value("projectName", chosenProjectFolder);

    context.scenePaths.clear();

    if (projectData.contains("scenes")) {
        for (const auto& scene : projectData["scenes"]) {
            context.scenePaths.push_back(projectFolder + scene.get<std::string>());
        }
    }

    if (context.scenePaths.empty()) {
        std::string scenesRoot = projectFolder + "assets/scenes/";
        if (fs::exists(scenesRoot)) {
            for (const auto& entry : fs::directory_iterator(scenesRoot)) {
                if (entry.is_regular_file() && entry.path().extension() == ".json") {
                    context.scenePaths.push_back(entry.path().string());
                }
            }
        }
    }

    std::string startScene = projectData.value("startingScene", "room_01");
    context.currentScenePath = projectFolder + "assets/scenes/" + startScene + ".json";

    if (!SceneLoader::loadFromFile(context.currentScenePath, context.scene)) {
        std::cout << "[Editor] ERROR: Could not load starting scene: " << context.currentScenePath << std::endl;
        return;
    }

    // IMPORTANT: sync viewport scene
    context.previewScene.loadFromData(context.scene);
    context.previewDirty = false;

    // IMPORTANT: rebuild project-local scene list
    context.scenePaths.clear();

    std::string scenesRoot = projectFolder + "assets/scenes/";
    if (fs::exists(scenesRoot)) {
        for (const auto& entry : fs::directory_iterator(scenesRoot)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                context.scenePaths.push_back(entry.path().string());
            }
        }
    }

    context.currentSceneIndex = 0;
    for (int i = 0; i < (int)context.scenePaths.size(); i++) {
        if (context.scenePaths[i] == context.currentScenePath) {
            context.currentSceneIndex = i;
            break;
        }
    }

    context.selection = {};
    context.sceneLoaded = true;
    context.dirty = false;

    std::cout << "[Editor] Successfully loaded project: " << context.projectName << std::endl;
    json settings;
    settings["lastProject"] = chosenProjectFolder;

    std::ofstream settingsFile("EditorSettings.json");
    settingsFile << settings.dump(4);
}

void SaveProject(EditorContext& context) {
    if (context.projectPath.empty()) {
        std::cout << "[Editor] ERROR: No project loaded. Create or load a project first." << std::endl;
        return;
    }

    if (!context.currentScenePath.empty()) {
        if (!SceneLoader::saveToFile(context.currentScenePath, context.scene)) {
            std::cout << "[Editor] ERROR: Failed to save current scene: " << context.currentScenePath << std::endl;
            return;
        }
    }

    context.scenePaths.clear();

    std::string scenesFolder = context.projectPath + "assets/scenes/";
    if (fs::exists(scenesFolder)) {
        for (const auto& entry : fs::directory_iterator(scenesFolder)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                context.scenePaths.push_back(entry.path().string());
            }
        }
    }

    json projectData;
    projectData["projectName"] = context.projectName;
    projectData["startingScene"] = fs::path(context.currentScenePath).stem().string();
    projectData["scenes"] = json::array();

    for (const auto& scenePath : context.scenePaths) {
        projectData["scenes"].push_back(fs::relative(scenePath, context.projectPath).generic_string());
    }

    std::ofstream file(context.projectPath + "project.json");
    if (!file.is_open()) {
        std::cout << "[Editor] ERROR: Could not write project.json." << std::endl;
        return;
    }

    file << projectData.dump(4);

    context.previewScene.loadFromData(context.scene);
    context.dirty = false;
    context.previewDirty = false;

    std::cout << "[Editor] Saved project: " << context.projectName << std::endl;
}
void RefreshProjectList(EditorContext& context) {
    context.availableProjects.clear();
    
    std::string projectsRoot = "Projects/";
    if (!fs::exists(projectsRoot)) {
        fs::create_directory(projectsRoot);
    }

    // Loop through every folder inside "Projects/"
    for (const auto& entry : fs::directory_iterator(projectsRoot)) {
        if (entry.is_directory()) {
            // This grabs just the name of the folder (e.g., "TestGame2")
            context.availableProjects.push_back(entry.path().filename().string());
        }
    }
}


void CreateNewProject(EditorContext& context, const std::string& folderName, const std::string& userProjectName)
{
    if (folderName.empty()) {
        std::cout << "[Editor] ERROR: Folder name is empty." << std::endl;
        return;
    }

    std::string projectFolder = "Projects/" + folderName + "/";

    if (fs::exists(projectFolder)) {
        std::cout << "[Editor] ERROR: Project already exists: " << projectFolder << std::endl;
        return;
    }

    fs::create_directories(projectFolder + "assets/scenes");

    context.projectPath = projectFolder;
    context.projectName = userProjectName.empty() ? folderName : userProjectName;
    context.currentScenePath = projectFolder + "assets/scenes/room_01.json";

    // create empty/default scene
    context.scene = SceneData{};
    context.scene.name = "room_01";

    SceneLoader::saveToFile(context.currentScenePath, context.scene);

    json projectJson;
    projectJson["projectName"] = context.projectName;
    projectJson["startingScene"] = "room_01";
    projectJson["scenes"] = json::array({ "assets/scenes/room_01.json" });

    std::ofstream file(projectFolder + "project.json");
    file << projectJson.dump(4);

    context.previewScene.loadFromData(context.scene);
    context.scenePaths.clear();
    context.scenePaths.push_back(context.currentScenePath);
    context.currentSceneIndex = 0;
    context.sceneLoaded = true;
    context.dirty = false;
    context.previewDirty = false;

    RefreshProjectList(context);

    std::cout << "[Editor] Created new empty project: " << context.projectName << std::endl;
}

void LoadLastProject(EditorContext& context)
{
    std::ifstream file("EditorSettings.json");
    if (!file.is_open())
        return;

    json settings;
    file >> settings;

    std::string lastProject = settings.value("lastProject", "");
    if (lastProject.empty())
        return;

    LoadProject(context, lastProject);
}