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
    
    // Dynamically build paths based on what the user clicked!
    std::string projectFolder = "Projects/" + chosenProjectFolder + "/";
    std::string settingsPath = projectFolder + "project.json";

    std::ifstream file(settingsPath);
    if (file.is_open()) {
        json projectData;
        file >> projectData;
        file.close();

        context.projectPath = projectFolder;
        if (projectData.contains("projectName")) {
            context.projectName = projectData["projectName"];
        }

        std::string startScene = projectData["startingScene"];
        context.currentScenePath = projectFolder + "assets/scenes/" + startScene + ".json";
        
        if (SceneLoader::loadFromFile(context.currentScenePath, context.scene)) {
            std::cout << "[Editor] Successfully loaded project: " << context.projectName << "!" << std::endl;
            context.selection = {};
            context.sceneLoaded = true;
            context.dirty = false;
        } else {
            std::cout << "[Editor] ERROR: Could not load starting scene!" << std::endl;
        }
    } else {
        std::cout << "[Editor] ERROR: Could not find project.json inside " << projectFolder << "!" << std::endl;
    }
}

void SaveProject(EditorContext& context) {
    if (context.projectPath.empty()) {
        std::cout << "[Editor] ERROR: No project loaded! Cannot save project configuration." << std::endl; // Changed \n
        return;
    }

    std::string settingsPath = context.projectPath + "project.json";
    std::cout << "[Editor] Saving project configuration to " << settingsPath << "..." << std::endl; // Changed \n

    json projectData;
    projectData["projectName"] = context.projectName;
    projectData["startingScene"] = context.scene.name; 

    std::ofstream file(settingsPath);
    if (file.is_open()) {
        file << projectData.dump(4);
        file.close();
        SceneLoader::saveToFile(context.currentScenePath, context.scene);
        context.dirty = false;
        std::cout << "[Editor] Project configuration saved successfully!" << std::endl; // Changed \n
    } else {
        std::cout << "[Editor] ERROR: Could not open project.json for writing!" << std::endl; // Changed \n
    }
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


void CreateNewProject(EditorContext& context, const std::string& folderName, const std::string& userProjectName) {
    // 1. Sanity check: Ensure the user didn't leave the name blank
    if (folderName.empty() || userProjectName.empty()) {
        std::cout << "[Editor] ERROR: Folder name or Project name cannot be empty!" << std::endl;
        return;
    }

    // 2. Define our paths
    std::string newProjectRoot = "Projects/" + folderName + "/";
    std::string scenesFolder = newProjectRoot + "assets/scenes/";

    // 3. Check if the project folder already exists so we don't accidentally wipe a game!
    if (fs::exists(newProjectRoot)) {
        std::cout << "[Editor] ERROR: A project folder named '" << folderName << "' already exists!" << std::endl;
        return;
    }

    // 4. Create the nested folders automatically (mkdir -p equivalent)
    fs::create_directories(scenesFolder);

    // 5. Generate the default project.json file contents
    json projectJson;
    projectJson["projectName"] = userProjectName;
    projectJson["startingScene"] = "room_01";

    std::ofstream projFile(newProjectRoot + "project.json");
    if (projFile.is_open()) {
        projFile << projectJson.dump(4);
        projFile.close();
    }

    // 6. Generate a baseline template room_01.json so the loader doesn't crash on an empty file
    json defaultSceneJson;
    defaultSceneJson["name"] = "room_01";
    defaultSceneJson["camera"] = {
        {"mode", "fixed"},
        {"position", {0, 10, 10}},
        {"target", {0, 0, 0}}
    };
    defaultSceneJson["playerSpawns"] = json::array({
        {{"id", "default"}, {"position", {0, 1, 0}}, {"skinChoice", 1}}
    });
    defaultSceneJson["cubes"] = json::array();
    defaultSceneJson["entities"] = json::array();

    std::ofstream sceneFile(scenesFolder + "room_01.json");
    if (sceneFile.is_open()) {
        sceneFile << defaultSceneJson.dump(4);
        sceneFile.close();
    }

    std::cout << "[Editor] Successfully created new project workspace: " << userProjectName << std::endl;
    
    // 7. Force the launcher window to scan the directory and reveal the new project instantly!
    RefreshProjectList(context);
}