#include "EditorApp.h"
#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"
#include "editor/gui/EditorUI.h"
#include "engine/scene/SceneLoader.h"
#include "editor/project/ProjectManager.h"
#include <filesystem>
#include "EditorPanels.h"

bool EditorApp::init()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "Virten Editor");
    SetTargetFPS(60);

    rlImGuiSetup(true);

    constexpr float editorUiScale = 1.75f;
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(editorUiScale);
    style.FontScaleMain = editorUiScale;

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    LoadComponentSchemas(context);

    context.sceneLoaded = SceneLoader::loadFromFile(context.currentScenePath, context.scene);

    context.editorCamera.position   = { 6.0f, 8.0f, 6.0f };
    context.editorCamera.target     = { 0.0f, 0.0f, 0.0f };
    context.editorCamera.up         = { 0.0f, 1.0f, 0.0f };
    context.editorCamera.fovy       = 45.0f;
    context.editorCamera.projection = CAMERA_PERSPECTIVE;

    context.viewportTexture = LoadRenderTexture(1280, 720);
    context.viewportReady   = (context.viewportTexture.texture.id != 0);
    
    /*context.resourceManager.LoadFromManifest("assets/assets.json");
    
    LoadLastProject(context);

    if (!context.sceneLoaded)
    {
        context.sceneLoaded = SceneLoader::loadFromFile(context.currentScenePath, context.scene);
        context.previewScene.load(context.currentScenePath);
        context.scene = context.previewScene.getData();

        namespace fs = std::filesystem;

        context.scenePaths.clear();

        if (fs::exists("assets/scenes"))
        {
            for (const auto& entry : fs::directory_iterator("assets/scenes"))
            {
                if (!entry.is_regular_file())
                    continue;

                std::string path = entry.path().string();

                if (entry.path().extension() == ".json")
                {
                    context.scenePaths.push_back(path);
                }
            }
        }

        context.currentSceneIndex = 0;

        for (int i = 0; i < (int)context.scenePaths.size(); i++)
        {
            if (context.scenePaths[i] == context.currentScenePath)
            {
                context.currentSceneIndex = i;
                break;
            }
        }
    }*/

    LoadLastProject(context);

    if (context.projectName.empty()) {
        RefreshProjectList(context);
    }
    
    return context.viewportReady;
}

void EditorApp::run()
{
    SetExitKey(KEY_NULL);

    while (!context.exitRequested)
    {
        if (WindowShouldClose())
        {
            RequestEditorExit(context);
        }

        if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_S))
        {
            SaveProject(context);
        }

        BeginDrawing();
        ClearBackground(DARKGRAY);

        rlImGuiBegin();
        DrawEditorUI(context);
        rlImGuiEnd();

        EndDrawing();
    }
}

void EditorApp::shutdown()
{
    if (context.viewportReady)
    {
        UnloadRenderTexture(context.viewportTexture);
        context.viewportReady = false;
    }

    rlImGuiShutdown();
    CloseWindow();
}