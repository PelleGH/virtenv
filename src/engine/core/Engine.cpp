#include "Engine.h"
#include "engine/scene/Scene.h"

#include <chrono>
#include <iostream>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;
namespace
{
    bool g_profilerEnabled = false;
    float g_profilerElapsed = 0.0f;
    int g_profilerFrames = 0;

    std::unordered_map<std::string, double> g_profilerTotalsMs;
    std::vector<std::string> g_profilerLastReport;

    void resetProfiler()
    {
        g_profilerElapsed = 0.0f;
        g_profilerFrames = 0;
        g_profilerTotalsMs.clear();
    }

    void beginProfilerFrame(float dt)
    {
        // F3 sometimes does not register on some keyboards/laptops,
        // so P is a backup toggle.
        if (IsKeyPressed(KEY_F3) || IsKeyPressed(KEY_P))
        {
            g_profilerEnabled = !g_profilerEnabled;
            resetProfiler();

            g_profilerLastReport.clear();
            g_profilerLastReport.push_back(
                g_profilerEnabled
                    ? "Profiler enabled"
                    : "Profiler disabled"
            );

            std::cout << "\n[Profiler] "
                      << (g_profilerEnabled ? "Enabled" : "Disabled")
                      << "\n";
        }

        if (!g_profilerEnabled)
            return;

        g_profilerElapsed += dt;
        g_profilerFrames++;
    }

    template <typename Func>
    void profileBlock(const std::string& name, Func&& func)
    {
        if (!g_profilerEnabled)
        {
            func();
            return;
        }

        auto start = std::chrono::steady_clock::now();

        func();

        auto end = std::chrono::steady_clock::now();

        double ms =
            std::chrono::duration<double, std::milli>(end - start).count();

        g_profilerTotalsMs[name] += ms;
    }

    void updateProfilerReportIfNeeded()
    {
        if (!g_profilerEnabled)
            return;

        if (g_profilerElapsed < 2.0f)
            return;

        g_profilerLastReport.clear();

        std::ostringstream header;
        header << "Virtenv profiler | Frames: "
               << g_profilerFrames
               << " | Avg FPS: "
               << std::fixed << std::setprecision(1)
               << (g_profilerFrames / g_profilerElapsed);

        g_profilerLastReport.push_back(header.str());

        std::vector<std::string> order =
        {
            "InputSystem",
            "InteractionSystem",
            "AISystem",
            "MovementSystem",
            "ProjectileSystem",
            "ConditionSystem",
            "CollisionSystem",
            "TriggerSystem",
            "DialogueManager",
            "EventBus",
            "CameraSystem",
            "SceneManager",
            "Scene cleanup",
            "RenderSystem",
            "InventoryUI",
            "Dialogue render"
        };

        for (const std::string& name : order)
        {
            auto it = g_profilerTotalsMs.find(name);
            if (it == g_profilerTotalsMs.end())
                continue;

            double averageMs = it->second / g_profilerFrames;

            std::ostringstream line;
            line << std::left << std::setw(18) << name
                 << ": "
                 << std::fixed << std::setprecision(4)
                 << averageMs
                 << " ms/frame";

            g_profilerLastReport.push_back(line.str());
        }

        // Also print to console, if a console is visible.
        std::cout << "\n========== Virtenv profiler ==========\n";
        for (const std::string& line : g_profilerLastReport)
            std::cout << line << "\n";
        std::cout << "======================================\n";

        resetProfiler();
    }

    void drawProfilerOverlay()
    {
        const int x = 10;
        int y = 10;
        const int fontSize = 18;
        const int lineHeight = 22;

        DrawText("Press F3 or P to toggle profiler", x, y, fontSize, DARKGRAY);
        y += lineHeight;

        if (!g_profilerEnabled)
        {
            DrawText("Profiler: OFF", x, y, fontSize, GRAY);
            return;
        }

        DrawText("Profiler: ON", x, y, fontSize, GREEN);
        y += lineHeight;

        for (const std::string& line : g_profilerLastReport)
        {
            DrawText(line.c_str(), x, y, fontSize, BLACK);
            y += lineHeight;
        }

        if (g_profilerLastReport.empty())
        {
            DrawText("Collecting data...", x, y, fontSize, BLACK);
        }
    }
}
static const std::string EDITOR_SETTINGS_PATH =
    "src/editor/config/EditorSettings.json";

static std::string getActiveProjectPath()
{
    std::ifstream file(EDITOR_SETTINGS_PATH);
    if (!file.is_open())
    {
        std::cout << "[Engine] No editor settings found at: "
                  << EDITOR_SETTINGS_PATH << std::endl;
        return "";
    }

    json settings;
    file >> settings;

    std::string lastProject = settings.value("lastProject", "");
    if (lastProject.empty())
    {
        std::cout << "[Engine] No lastProject in editor settings." << std::endl;
        return "";
    }

    std::string projectPath = "Projects/" + lastProject + "/";
    if (!fs::exists(projectPath + "project.json"))
    {
        std::cout << "[Engine] Project not found: " << projectPath << std::endl;
        return "";
    }

    std::cout << "[Engine] Active project: " << projectPath << std::endl;
    return projectPath;
}

bool Engine::init(const std::string& overrideRoot)
{
    InitWindow(1280, 720, "Virtenv");
    SetTargetFPS(60);

    std::string activeProjectPath = overrideRoot.empty() ? getActiveProjectPath() : overrideRoot;
    assetRoot = activeProjectPath.empty() ? "" : activeProjectPath;
    resourceManager.SetAssetRoot(assetRoot + "assets/");

    //Reads and creates the models and texture for the world
    resourceManager.LoadFromManifest("assets.json");
    sceneManager.init(resourceManager);

    cameraSystem.init(camera);

    questManager.setConditionManager(&conditionManager);
    questManager.loadQuests(assetRoot + "assets/quests.json");
    dialogueManager.setQuestManager(&questManager);

    eventBus.subscribe<SceneTransitionEvent>([this](const SceneTransitionEvent& event)
    {
        sceneManager.requestSceneChange(
            assetRoot + "assets/scenes/" + event.targetScene + ".json",
            event.targetSpawn
        );
    });

    eventBus.subscribe<DeathEvent>([this](const DeathEvent& event)
    {
        Scene& currentScene = sceneManager.getCurrentScene();
        ComponentStorage& components = currentScene.getComponentStorage();

        // Check if the entity that died is the player
        if (components.HasComponent<PlayerInput>(event.entity))
        {
            std::cout << "Player died! Reloading scene...\n";
            
            // Get the current scene's name (e.g., "room_01")
            std::string sceneName = currentScene.getData().name;
            
            // Queue a reload of the current scene, spawning at the default location
            sceneManager.requestPlayerRespawn();    
        }
        else
        {
            // It's just a normal enemy/cube, destroy it
            currentScene.queueEntityDestruction(event.entity);
        }
    });
    eventBus.subscribe<EnemyKilledEvent>([this](const EnemyKilledEvent& event)
    {
        GameplayEvent gameplayEvent;
        gameplayEvent.type = "enemy_killed";
        gameplayEvent.targetId = event.enemyType;
        gameplayEvent.amount = 1;

        eventBus.publish(gameplayEvent);
    });

    eventBus.subscribe<GameplayEvent>([this](const GameplayEvent& event)
    {
        questManager.onEvent(event.type, event.targetId, event.amount);
    });

    // Interaction type
    actionHandlers["StartDialogue"] = [this](const std::string& data){
        this -> dialogueManager.startDialogue(data);
    };

    eventBus.subscribe<ActionEvent>([this](const ActionEvent& actionEvent) {
        this->onActionEvent(actionEvent);
    });

    eventBus.subscribe<SpawnItemDropEvent>([this](const SpawnItemDropEvent& event)
    {
        sceneManager.getCurrentScene().spawnItemDrop(event.x, event.y, event.z, event.itemId);
    });
    
    running = true;

    std::string startScene = "room_01";

    if (!activeProjectPath.empty())
    {
        std::ifstream projectFile(activeProjectPath + "project.json");
        if (projectFile.is_open())
        {
            json projectData;
            projectFile >> projectData;
            startScene = projectData.value("startingScene", "room_01");
        }
    }

    bool sceneLoaded = sceneManager.loadScene(assetRoot + "assets/scenes/" + startScene + ".json");

    resourceManager.loadItems("items.json");
    inventorySystem.initialize(sceneManager.getCurrentScene(), eventBus, resourceManager);

    combatSystem.initialize(sceneManager.getCurrentScene(), eventBus);
    projectileSystem.initialize(sceneManager.getCurrentScene(), eventBus);

    return sceneLoaded;
}

void Engine::run()
{
    using clock = std::chrono::steady_clock;

    auto lastTime = clock::now(); // get time in whatever format the clock uses

    while (running && !WindowShouldClose())
    {
        auto currentTime = clock::now();
        std::chrono::duration<float> elapsed = currentTime - lastTime; // convert result to seconds as float inside duration wrapper
        lastTime = currentTime;

        float dt = elapsed.count(); // get the float value from the duration wrapper

        update(dt);
        render();
    }
}

void Engine::update(float dt)
{
    beginProfilerFrame(dt);

    bool gameplayPaused = dialogueManager.isActive();

    profileBlock("InputSystem", [this]()
    {
        inputSystem.update(sceneManager.getCurrentScene(), eventBus);
    });

    if (IsKeyPressed(KEY_K))
    {
        questManager.onEvent("enemy_killed", "enemy", 1);
    }

    if (!gameplayPaused)
    {
        profileBlock("InteractionSystem", [this]()
        {
            interactionSystem.update(sceneManager.getCurrentScene(), eventBus);
        });

        profileBlock("AISystem", [this, dt]()
        {
            aiSystem.update(sceneManager.getCurrentScene(), eventBus, dt);
        });

        profileBlock("MovementSystem", [this, dt]()
        {
            movementSystem.update(sceneManager.getCurrentScene(), dt);
        });

        profileBlock("ProjectileSystem", [this, dt]()
        {
            projectileSystem.update(sceneManager.getCurrentScene(), dt);
        });

        profileBlock("ConditionSystem", [this]()
        {
            conditionalSystem.update(
                sceneManager.getCurrentScene(),
                conditionManager
            );
        });

        profileBlock("CollisionSystem", [this]()
        {
            collisionSystem.update(sceneManager.getCurrentScene(), eventBus);
        });

        profileBlock("TriggerSystem", [this]()
        {
            triggerSystem.update(
                sceneManager.getCurrentScene(),
                eventBus
            );
        });
    }

    profileBlock("DialogueManager", [this]()
    {
        dialogueManager.update();
    });

    profileBlock("EventBus", [this]()
    {
        eventBus.dispatch();
    });

    profileBlock("CameraSystem", [this]()
    {
        cameraSystem.update(sceneManager.getCurrentScene(), camera);
    });

    if (IsKeyPressed(KEY_NINE))
    {
        sceneManager.getCurrentScene().saveState();
    }

    if (IsKeyPressed(KEY_Q))
    {
        conditionManager.debugUnlocked =
            !conditionManager.debugUnlocked;
    }

    profileBlock("SceneManager", [this, dt]()
    {
        sceneManager.update(dt);
        sceneManager.applyPendingSceneChange();
    });

    profileBlock("Scene cleanup", [this]()
    {
        sceneManager.getCurrentScene().cleanupDestroyedEntities();
    });
}

void Engine::render()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    profileBlock("RenderSystem", [this]()
    {
        renderSystem.render(sceneManager.getCurrentScene(), resourceManager, camera);
    });

    profileBlock("InventoryUI", [this]()
    {
        inventoryUISystem.render(sceneManager.getCurrentScene(), resourceManager);
    });

    profileBlock("Dialogue render", [this]()
    {
        dialogueManager.render();
    });

    updateProfilerReportIfNeeded();
    drawProfilerOverlay();

    EndDrawing();
}
void Engine::shutdown()
{
    sceneManager.shutdown();
    CloseWindow();
    std::cout << "Engine shutdown\n";
}

void Engine::debugNPCInteraction()
{
    if (dialogueManager.isActive())
        return;

    Scene& scene = sceneManager.getCurrentScene();
    ComponentStorage& storage = scene.getComponentStorage();

    auto& players = storage.GetComponents<PlayerInput>();
    auto& npcs = storage.GetComponents<DialogueSource>();

    if (players.empty())
        return;

    if (!IsKeyPressed(KEY_E))
        return;

    Entity player = players.begin()->first;

    if (!storage.HasComponent<TransformComponent>(player))
        return;

    TransformComponent& playerTransform =
        storage.GetComponent<TransformComponent>(player);

    for (auto& [npc, dialogue] : npcs)
    {
        if (!storage.HasComponent<TransformComponent>(npc))
            continue;

        TransformComponent& npcTransform =
            storage.GetComponent<TransformComponent>(npc);

        float dx = playerTransform.x - npcTransform.x;
        float dz = playerTransform.z - npcTransform.z;

        float distSq = dx * dx + dz * dz;

        if (distSq < 2.25f)
        {
            dialogueManager.startDialogue(
                dialogue.dialogueSetId
            );

            break;
        }
    }
}

void Engine::onActionEvent(const ActionEvent& actionEvent){

    //IGNORE PICKUPS
    if (actionEvent.actionType == "PickupItem") return;

    //Find the action
    auto it = actionHandlers.find(actionEvent.actionType);

    //If the action excist
    if (it != actionHandlers.end()){
        it -> second(actionEvent.targetData);
    }else{
       std::cout << "VARNING: Unknown actionType from JSON: " << actionEvent.actionType << '\n';
    }
}