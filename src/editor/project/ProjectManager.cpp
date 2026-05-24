#include "ProjectManager.h"

#include "engine/scene/SceneLoader.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

static const std::string EDITOR_SETTINGS_PATH =
    "src/editor/config/EditorSettings.json";

namespace fs = std::filesystem;
using json = nlohmann::json;

void LoadProject(EditorContext& context, const std::string& chosenProjectFolder) {
    std::cout << "[Editor] Loading Project..." << std::endl;

    // Clear memory from old models and textures
    context.resourceManager.clear();

    std::string projectFolder = "Projects/" + chosenProjectFolder + "/";
    // Update the search way to project
    context.resourceManager.SetAssetRoot(projectFolder + "assets/");

    // Load the new projects assets json
    context.resourceManager.LoadFromManifest("assets.json");

    // Load our items
    context.resourceManager.loadItems("items.json");

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
    context.buildOutdated = projectData.value("buildOutdated", true);
    SaveEditorSettings(context);
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

    context.startingScene = projectData.value("startingScene", "room_01");
    context.currentScenePath = projectFolder + "assets/scenes/" + context.startingScene + ".json";

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
    context.gridPainterUndoStack.clear();

    SaveEditorSettings(context);
    
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

    if (context.startingScene.empty())
    {
        context.startingScene = fs::path(context.currentScenePath).stem().string();
    }

    projectData["startingScene"] = context.startingScene;
    projectData["buildOutdated"] = context.buildOutdated;
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
    
    json assetManifest;
    assetManifest["textures"] = json::array();
    assetManifest["cube_models"] = json::array();
    assetManifest["3Dmodels"] = json::array();

    std::ofstream assetsFile(projectFolder + "assets/assets.json");
    assetsFile << assetManifest.dump(4);

    json itemsJson;
    itemsJson["items"] = json::array();

    std::ofstream itemsFile(projectFolder + "assets/items.json");
    itemsFile << itemsJson.dump(4);

    json questsJson;
    questsJson["quests"] = json::array();

    std::ofstream questsFile(projectFolder + "assets/quests.json");
    questsFile << questsJson.dump(4);
    context.projectPath = projectFolder;
    context.projectName = userProjectName.empty() ? folderName : userProjectName;
    context.currentScenePath = projectFolder + "assets/scenes/room_01.json";
    context.startingScene = "room_01";
    context.buildOutdated = true;

    // create empty/default scene
    context.scene = SceneData{};
    context.scene.name = "room_01";

    SceneLoader::saveToFile(context.currentScenePath, context.scene);

    json projectJson;
    projectJson["projectName"] = context.projectName;
    projectJson["startingScene"] = "room_01";
    projectJson["buildOutdated"] = context.buildOutdated;
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
    SaveEditorSettings(context);
    std::cout << "[Editor] Created new empty project: " << context.projectName << std::endl;
}

void LoadLastProject(EditorContext& context)
{
    LoadEditorSettings(context);

    std::ifstream file(EDITOR_SETTINGS_PATH);
    if (!file.is_open())
        return;

    json settings;
    file >> settings;

    std::string lastProject = settings.value("lastProject", "");
    if (lastProject.empty())
        return;

    LoadProject(context, lastProject);
}

void BuildProject(EditorContext& context, const std::string& outputDir)
{
    if (context.projectPath.empty()) {
        std::cout << "[Builder] ERROR: No project loaded.\n";
        return;
    }

    try {
        std::cout << "[Builder] Auto-saving project before build...\n";
        SaveProject(context);

        fs::create_directories(outputDir);

        std::cout << "[Builder] Compiling Runtime... This might take a moment.\n";
        
        // This command tells CMake to build just the "Runtime" target in Debug mode
        int compileResult = std::system("cmake --build build --config Debug --target Runtime");
        
        if (compileResult != 0) {
            std::cout << "[Builder] ERROR: CMake compilation failed! Check your terminal for errors.\n";
            return; // Abort the export process if compilation fails
        }
        
        std::cout << "[Builder] Compilation successful! Packaging files...\n";

        // Copy and rename Runtime.exe
        std::string runtimeSource = "build/Debug/Runtime.exe"; 
        std::string exeDestination = outputDir + "/" + context.projectName + ".exe";
        
        if (fs::exists(runtimeSource)) {
            fs::copy_file(runtimeSource, exeDestination, fs::copy_options::overwrite_existing);
        } else {
            std::cout << "[Builder] ERROR: Runtime.exe not found! Compile RuntimeMain.cpp first.\n";
            return;
        }

        // Copy ALL .dll files from the build folder so the game doesn't crash on other PCs
        std::string buildFolder = "build/Debug"; // Change this if your DLLs are in a different folder
        if (fs::exists(buildFolder)) {
            for (const auto& entry : fs::directory_iterator(buildFolder)) {
                if (entry.is_regular_file() && entry.path().extension() == ".dll") {
                    std::string dllDest = outputDir + "/" + entry.path().filename().string();
                    fs::copy_file(entry.path(), dllDest, fs::copy_options::overwrite_existing);
                }
            }
        }

        // Copy project.json
        if (fs::exists(context.projectPath + "project.json")) {
            fs::copy_file(context.projectPath + "project.json", 
                          outputDir + "/project.json", 
                          fs::copy_options::overwrite_existing);
        }

        // Copy the project's assets folder
        std::string assetsSrc = context.projectPath + "assets";
        std::string assetsDest = outputDir + "/assets";
        
        if (fs::exists(assetsSrc)) {
            fs::copy(assetsSrc, assetsDest, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
        }

        std::cout << "[Builder] Build successful! Game at: " << exeDestination << "\n";

        context.buildOutdated = false;
        SaveProject(context);
    }
    catch (const fs::filesystem_error& e) {
        std::cout << "[Builder] File system error: " << e.what() << '\n';
    }
}

void SaveEditorSettings(EditorContext& context)
{
    json settings;

    std::string lastProject = "";

    if (!context.projectPath.empty())
    {
        fs::path projectPath = fs::path(context.projectPath).lexically_normal();

        if (projectPath.has_filename())
        {
            lastProject = projectPath.filename().string();
        }
        else
        {
            lastProject = projectPath.parent_path().filename().string();
        }
    }

    settings["lastProject"] = lastProject;

    settings["cubeTemplates"] = json::array();

    for (int i = 0; i < 5; i++)
    {
        const CubeTemplate& t = context.cubeTemplates[i];

        settings["cubeTemplates"].push_back({
            { "name", t.name },
            { "type", t.type },
            { "solid", t.solid },
            { "trigger", t.trigger },
            { "visible", t.visible },
            { "targetScene", t.targetScene },
            { "targetSpawn", t.targetSpawn },
            { "modelID", t.modelID }
        });
    }

    std::ofstream settingsFile(EDITOR_SETTINGS_PATH);
    settingsFile << settings.dump(4);
}

void LoadEditorSettings(EditorContext& context)
{
    std::ifstream file(EDITOR_SETTINGS_PATH);

    if (!file.is_open())
        return;

    json settings;
    file >> settings;

    if (!settings.contains("cubeTemplates"))
        return;

    const auto& templatesJson = settings["cubeTemplates"];

    for (int i = 0; i < 5 && i < (int)templatesJson.size(); i++)
    {
        const auto& t = templatesJson[i];

        context.cubeTemplates[i].name = t.value("name", context.cubeTemplates[i].name);
        context.cubeTemplates[i].type = t.value("type", context.cubeTemplates[i].type);
        context.cubeTemplates[i].solid = t.value("solid", context.cubeTemplates[i].solid);
        context.cubeTemplates[i].trigger = t.value("trigger", context.cubeTemplates[i].trigger);
        context.cubeTemplates[i].visible = t.value("visible", context.cubeTemplates[i].visible);
        context.cubeTemplates[i].targetScene = t.value("targetScene", context.cubeTemplates[i].targetScene);
        context.cubeTemplates[i].targetSpawn = t.value("targetSpawn", context.cubeTemplates[i].targetSpawn);
        context.cubeTemplates[i].modelID = t.value("modelID", context.cubeTemplates[i].modelID);
    }
}
void CreateNewScene(EditorContext& context, const std::string& sceneName)
{
    if (context.projectPath.empty())
    {
        std::cout << "[Editor] ERROR: No project loaded." << std::endl;
        return;
    }

    if (sceneName.empty())
    {
        std::cout << "[Editor] ERROR: Scene name is empty." << std::endl;
        return;
    }

    std::string scenesFolder = context.projectPath + "assets/scenes/";
    fs::create_directories(scenesFolder);

    std::string scenePath = scenesFolder + sceneName + ".json";

    if (fs::exists(scenePath))
    {
        std::cout << "[Editor] ERROR: Scene already exists: " << scenePath << std::endl;
        return;
    }

    SceneData newScene;
    newScene.name = sceneName;

    newScene.camera.mode = "followPlayer";
    newScene.camera.positionX = 0.0f;
    newScene.camera.positionY = 10.0f;
    newScene.camera.positionZ = 10.0f;
    newScene.camera.targetX = 0.0f;
    newScene.camera.targetY = 0.0f;
    newScene.camera.targetZ = 0.0f;

    PlayerSpawn defaultSpawn;
    defaultSpawn.x = 0.0f;
    defaultSpawn.y = 1.0f;
    defaultSpawn.z = 0.0f;
    defaultSpawn.skinChoice = 0;

    newScene.playerSpawns["default"] = defaultSpawn;

    if (!SceneLoader::saveToFile(scenePath, newScene))
    {
        std::cout << "[Editor] ERROR: Failed to create scene: " << scenePath << std::endl;
        return;
    }

    context.scenePaths.push_back(scenePath);
    context.currentScenePath = scenePath;
    context.currentSceneIndex = (int)context.scenePaths.size() - 1;

    context.scene = newScene;
    context.previewScene.loadFromData(context.scene);

    context.selection = {};
    context.sceneLoaded = true;
    context.dirty = true;
    context.previewDirty = false;
    context.gridPainterUndoStack.clear();

    SaveProject(context);

    std::cout << "[Editor] Created new scene: " << sceneName << std::endl;
}