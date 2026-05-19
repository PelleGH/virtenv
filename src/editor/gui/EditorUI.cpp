#include "EditorUI.h"
#include "imgui.h"
#include "../src/engine/scene/SceneManager.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "../src/engine/ecs/EntityFactory.h"
#include <cstring>
#include "../../engine/scene/SceneLoader.h"

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

static void DrawHierarchy(EditorContext& context)
{
    ImGui::Begin("Hierarchy");

    if (!context.sceneLoaded)
    {
        ImGui::Text("No scene loaded.");
        ImGui::End();
        return;
    }

    ImGui::Text("Scene: %s", context.scene.name.c_str());

    if (ImGui::TreeNode("Static Grid Cubes"))
    {
        for (int i = 0; i < (int)context.scene.cubes.size(); i++)
        {
            const auto& cube = context.scene.cubes[i];

            char label[128];
            snprintf(label, sizeof(label), "%s (%d, %d, %d)",
                cube.type.c_str(),
                cube.position.x,
                cube.position.y,
                cube.position.z
            );

            bool selected =
                context.selection.type == SelectionType::GridCube &&
                context.selection.index == i;

            if (ImGui::Selectable(label, selected))
            {
                context.selection.type = SelectionType::GridCube;
                context.selection.index = i;
            }
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Entities"))
    {
        for (int i = 0; i < (int)context.scene.entities.size(); i++)
        {
            auto& entity = context.scene.entities[i];

            std::string displayName;

            if (entity.contains("name") && entity["name"].is_string())
            {
                displayName = entity["name"];
            }
            else
            {
                displayName = "Unnamed Entity";
            }

            char label[128];

            snprintf(label,
                sizeof(label),
                "%s##entity_%d",
                displayName.c_str(),
                i
            );

            bool selected =
                context.selection.type == SelectionType::Entity &&
                context.selection.index == i;

            if (ImGui::Selectable(label, selected))
            {
                context.selection.type = SelectionType::Entity;
                context.selection.index = i;
            }
        }

        ImGui::TreePop();
    }

    ImGui::End();
}

static void DrawInspector(EditorContext& context)
{
    ImGui::Begin("Inspector");

    if (context.selection.type == SelectionType::None)
    {
        ImGui::Text("Nothing selected.");
        ImGui::End();
        return;
    }

    if (context.selection.type == SelectionType::GridCube)
    {
        int index = context.selection.index;

        if (index < 0 || index >= (int)context.scene.cubes.size())
        {
            ImGui::Text("Invalid cube selection.");
            ImGui::End();
            return;
        }

        auto& cube = context.scene.cubes[index];

        ImGui::Text("Grid Cube");

        if (ImGui::InputInt("X", &cube.position.x)) context.dirty = true;
        if (ImGui::InputInt("Y", &cube.position.y)) context.dirty = true;
        if (ImGui::InputInt("Z", &cube.position.z)) context.dirty = true;

        const char* types[] = { "floor", "wall", "door", "trigger" };

        int currentType = 0;

        for (int i = 0; i < 4; i++)
        {
            if (cube.type == types[i])
                currentType = i;
        }

        if (ImGui::Combo("Type", &currentType, types, IM_ARRAYSIZE(types)))
        {
            cube.type = types[currentType];

            if (cube.type == "floor")
            {
                cube.solid = false;
                cube.trigger = false;
            }
            else if (cube.type == "wall")
            {
                cube.solid = true;
                cube.trigger = false;
            }
            else if (cube.type == "trigger")
            {
                cube.solid = false;
                cube.trigger = true;
            }

            context.dirty = true;
        }

        if (ImGui::Checkbox("Solid", &cube.solid)) context.dirty = true;
        if (ImGui::Checkbox("Trigger", &cube.trigger)) context.dirty = true;
        if (ImGui::Button("Duplicate Cube"))
            {
                GridCube copy = cube;
                copy.position.x += 1;

                context.scene.cubes.push_back(copy);

                context.selection.type = SelectionType::GridCube;
                context.selection.index = (int)context.scene.cubes.size() - 1;

                context.dirty = true;
            }

            if (ImGui::Button("Delete Cube"))
            {
                context.scene.cubes.erase(context.scene.cubes.begin() + index);

                context.selection.type = SelectionType::None;
                context.selection.index = -1;

                context.dirty = true;

                ImGui::End();
                return;
            }
    }

    if (context.selection.type == SelectionType::Entity)
    {
        int index = context.selection.index;

        if (index < 0 || index >= (int)context.scene.entities.size())
        {
            ImGui::Text("Invalid entity selection.");
            ImGui::End();
            return;
        }

        ImGui::Text("Entity JSON");
        std::string currentName = "Unnamed Entity";

        if (context.scene.entities[index].contains("name"))
        {
            currentName = context.scene.entities[index]["name"];
        }

        char nameBuffer[128];
        strncpy(nameBuffer, currentName.c_str(), sizeof(nameBuffer));
        nameBuffer[sizeof(nameBuffer) - 1] = '\0';

        if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
        {
            context.scene.entities[index]["name"] = nameBuffer;
            context.dirty = true;
        }
        ImGui::TextWrapped("%s", context.scene.entities[index].dump(2).c_str());
    }

    ImGui::End();
}
static void DrawDockspace()
{
    static bool dockspaceOpen = true;

    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoBackground;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("Editor Dockspace", &dockspaceOpen, windowFlags);

    ImGui::PopStyleVar(3);
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save Scene"))
            {
                // call save here later
            }

            if (ImGui::MenuItem("Load Scene"))
            {
                // call load here later
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit"))
            {
                // handle exit later
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            ImGui::MenuItem("Undo", "Ctrl+Z", false, false);
            ImGui::MenuItem("Redo", "Ctrl+Y", false, false);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Hierarchy");
            ImGui::MenuItem("Inspector");
            ImGui::MenuItem("Assets");
            ImGui::MenuItem("Viewport");
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGuiID dockspaceID = ImGui::GetID("MainDockspace");
    ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f));

    ImGui::End();
}
void DrawEditorUI(EditorContext& context)
{
    DrawDockspace();
    ImGui::Begin("Assets");
    if (ImGui::Button("Add Floor Cube"))
        {
            GridCube cube;
            cube.type = "floor";
            cube.position = {0, 0, 0};
            cube.solid = false;
            cube.trigger = false;

            context.scene.cubes.push_back(cube);

            context.selection.type = SelectionType::GridCube;
            context.selection.index = (int)context.scene.cubes.size() - 1;

            context.dirty = true;
        }

        if (ImGui::Button("Add Wall Cube"))
        {
            GridCube cube;
            cube.type = "wall";
            cube.position = {0, 0, 0};
            cube.solid = true;
            cube.trigger = false;

            context.scene.cubes.push_back(cube);

            context.selection.type = SelectionType::GridCube;
            context.selection.index = (int)context.scene.cubes.size() - 1;

            context.dirty = true;
        }

    ImGui::Button("NPC");
    ImGui::End();

    ImGui::Begin("Viewport");
    ImGui::Text("Scene view later.");
    ImGui::Text("Cubes loaded: %d", (int)context.scene.cubes.size());
    ImGui::Text("Entities loaded: %d", (int)context.scene.entities.size());
    ImGui::End();

    DrawHierarchy(context);
    DrawInspector(context);
}