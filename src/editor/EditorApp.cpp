#include "EditorApp.h"
#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"
#include "editor/gui/EditorUI.h"
#include "engine/scene/SceneLoader.h"

bool EditorApp::init()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "Virten Editor");
    SetTargetFPS(60);

    rlImGuiSetup(true);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    context.sceneLoaded = SceneLoader::loadFromFile(context.currentScenePath, context.scene);

    context.editorCamera.position   = { 6.0f, 8.0f, 6.0f };
    context.editorCamera.target     = { 0.0f, 0.0f, 0.0f };
    context.editorCamera.up         = { 0.0f, 1.0f, 0.0f };
    context.editorCamera.fovy       = 45.0f;
    context.editorCamera.projection = CAMERA_PERSPECTIVE;

    context.viewportTexture = LoadRenderTexture(1280, 720);
    context.viewportReady   = (context.viewportTexture.texture.id != 0);
    return context.viewportReady;
}

void EditorApp::run()
{
    while (!WindowShouldClose())
    {
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