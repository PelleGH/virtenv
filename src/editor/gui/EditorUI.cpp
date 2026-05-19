#include "EditorUI.h"
#include "imgui.h"
#include "../src/engine/scene/SceneManager.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "../src/engine/ecs/EntityFactory.h"

using json = nlohmann::json;

void SaveCurrentScene(SceneManager& sceneManager) {
    // Make sure a project is actually loaded first!
    if (sceneManager.getProjectPath().empty()) {
        std::cout << "[Editor] ERROR: No project loaded! Cannot save.\n";
        return;
    }

    Scene& currentScene = sceneManager.getCurrentScene();
    std::string sceneName = currentScene.getData().name; // Gets "room_01"
    
    // Build the save path: "Projects/TestGame/assets/scenes/room_01.json"
    std::string savePath = sceneManager.getProjectPath() + "assets/scenes/" + sceneName + ".json";
    
    std::cout << "[Editor] Saving scene to " << savePath << "...\n";

    // --- REBUILT JSON SERIALIZATION LOGIC ---
    json sceneJson;
    sceneJson["name"] = sceneName; 
    
    ComponentStorage& components = currentScene.getComponentStorage();
    EntityFactory factory(currentScene.getEntityManager(), components);

    // 1. Serialize all active entities (skipping the player)
    json entitiesArray = json::array();
    for (Entity e : currentScene.getActiveEntities()) {
        if (!components.HasComponent<PlayerInput>(e)) {
            json entityData = factory.serialize(e);
            if (!entityData.empty()) {
                entitiesArray.push_back(entityData);
            }
        }
    }
    sceneJson["entities"] = entitiesArray;

    // 2. Serialize static cubes/walls
    json cubesArray = json::array();
    for (const auto& cube : currentScene.getData().cubes) {
        json cubeJson;
        cubeJson["position"] = { cube.position.x, cube.position.y, cube.position.z };
        cubeJson["type"] = cube.type;
        cubeJson["solid"] = cube.solid;
        cubeJson["trigger"] = cube.trigger;

        if (cube.type == "door") {
            cubeJson["targetScene"] = cube.targetScene;
            cubeJson["targetSpawn"] = cube.targetSpawn;
        }
        cubesArray.push_back(cubeJson);
    }
    sceneJson["cubes"] = cubesArray;
    // ----------------------------------------
    
    // Write the file to the new dynamic path!
    std::ofstream outFile(savePath);
    if (outFile.is_open()) {
        outFile << sceneJson.dump(4); 
        outFile.close();
        std::cout << "[Editor] Save successful!\n";
    } else {
        std::cout << "[Editor] ERROR: Could not open file for writing!\n";
    }
}

void LoadProject(SceneManager& sceneManager) {
    std::cout << "[Editor] Loading Project...\n";
    
    // 1. The path to the project file (Later, a file browser will provide this)
    std::string projectFolder = "Projects/TestGame/";
    std::string settingsPath = projectFolder + "project.json";

    // 2. Open and read the project.json file
    std::ifstream file(settingsPath);
    if (file.is_open()) {
        json projectData;
        file >> projectData;
        file.close();

        // 3. Lock the SceneManager into this folder!
        sceneManager.setProjectPath(projectFolder);

        // 4. Find the starting scene and load it
        std::string startScene = projectData["startingScene"];
        std::string fullScenePath = projectFolder + "assets/scenes/" + startScene + ".json";
        
        if (sceneManager.loadScene(fullScenePath)) {
            std::cout << "[Editor] Successfully loaded project: " << projectData["projectName"] << "!\n";
        } else {
            std::cout << "[Editor] ERROR: Could not load starting scene!\n";
        }
    } else {
        std::cout << "[Editor] ERROR: Could not find project.json!\n";
    }
}

void SetupEditorStyle()
{
    ImGui::StyleColorsDark();
}

void DrawEditorUI(SceneManager& sceneManager)
{
    //TOP MENU BAR
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            // Pass the scene manager into the functions when clicked!
            if (ImGui::MenuItem("Load Project", "Ctrl+O")) {
                LoadProject(sceneManager);
            }
            
            if (ImGui::MenuItem("Save Project", "Ctrl+S")) {
                // FIXED: Now correctly calls SaveCurrentScene!
                SaveCurrentScene(sceneManager); 
            }
            
            ImGui::Separator();
            
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                std::cout << "Exit clicked!\n";
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    ImGui::Begin("Assets");
    ImGui::Button("tile");
    ImGui::Button("NPC");
    ImGui::End();

    ImGui::Begin("Viewport");
    ImGui::Text("Scene view here");
    ImGui::End();

    ImGui::Begin("Inspector");
    ImGui::Text("Selected Object: Door");
    ImGui::End();
}