#include "../EditorPanels.h"
#include "imgui.h"
#include "raylib.h"
#include "rlImGui.h"
#include "raymath.h"

#include <cmath>
#include <cfloat>

static void RenderSceneViewport(EditorContext& context);
static void DrawEditorSelectionOverlay(EditorContext& context);

static EditorSelection PickViewportSelection(EditorContext& context, Ray ray);
static bool GetEntityPosition(const nlohmann::json& entity, Vector3& outPos);
static BoundingBox GetEntityBounds(const nlohmann::json& entity);
static void DrawEntityEditorOverlays(EditorContext& context);

static Camera3D BuildScenePreviewCamera(EditorContext& context);
static Vector3 GetCameraPreviewPlayerPosition(EditorContext& context);
static void DrawInvisibleCubeEditorOverlays(EditorContext& context);
static void DrawInvisibleCubeEditorOverlays(EditorContext& context)
{
    for (const auto& cube : context.scene.cubes)
    {
        if (cube.visible)
            continue;

        Vector3 pos = {
            (float)cube.position.x,
            (float)cube.position.y,
            (float)cube.position.z
        };

        Color color = cube.solid ? SKYBLUE : PURPLE;

        DrawCube(pos, 1.0f, 1.0f, 1.0f, Fade(color, 0.18f));
        DrawCubeWires(pos, 1.0f, 1.0f, 1.0f, color);
    }
}
static bool GetViewportMouseRay(
    
    EditorContext& context,
    Ray& outRay,
    ImVec2& outImageMin,
    ImVec2& outImageMax
);

static bool GetGridCellFromRay(
    EditorContext& context,
    Ray ray,
    int& outX,
    int& outY,
    int& outZ
);
static bool GetViewportMouseRay(
    EditorContext& context,
    Ray& outRay,
    ImVec2& outImageMin,
    ImVec2& outImageMax
)
{
    outImageMin = ImGui::GetItemRectMin();
    outImageMax = ImGui::GetItemRectMax();

    Vector2 mousePos = GetMousePosition();

    Vector2 relativePos = {
        mousePos.x - outImageMin.x,
        mousePos.y - outImageMin.y
    };

    float imageWidth = outImageMax.x - outImageMin.x;
    float imageHeight = outImageMax.y - outImageMin.y;

    if (relativePos.x < 0 || relativePos.y < 0 ||
        relativePos.x > imageWidth || relativePos.y > imageHeight)
    {
        return false;
    }

    Vector2 textureMouse = {
        relativePos.x * context.viewportTexture.texture.width / imageWidth,
        relativePos.y * context.viewportTexture.texture.height / imageHeight
    };

    Camera3D activeCamera = context.previewSceneCamera
        ? BuildScenePreviewCamera(context)
        : context.editorCamera;

    outRay = GetScreenToWorldRayEx(
        textureMouse,
        activeCamera,
        context.viewportTexture.texture.width,
        context.viewportTexture.texture.height
    );

    return true;
}

static bool GetGridCellFromRay(
    EditorContext& context,
    Ray ray,
    int& outX,
    int& outY,
    int& outZ
)
{
    float planeY = (float)context.gridPainterLayerY;

    if (fabsf(ray.direction.y) < 0.0001f)
        return false;

    float t = (planeY - ray.position.y) / ray.direction.y;

    if (t < 0.0f)
        return false;

    Vector3 hit = Vector3Add(
        ray.position,
        Vector3Scale(ray.direction, t)
    );

    outX = (int)roundf(hit.x);
    outY = context.gridPainterLayerY;
    outZ = (int)roundf(hit.z);

    return true;
}
void DrawViewport(EditorContext& context)
{
    ImGui::Begin("Viewport");
    ImGui::Checkbox("Preview Scene Camera", &context.previewSceneCamera);

    if (context.previewSceneCamera)
    {
        ImGui::SameLine();
        ImGui::TextDisabled("Using Scene Settings camera");
    }
    else
    {
        ImGui::SameLine();
        ImGui::TextDisabled("Using editor camera");
    }
    ImVec2 size = ImGui::GetContentRegionAvail();

    if (size.x > 1 && size.y > 1 &&
        ((int)size.x != context.viewportTexture.texture.width ||
        (int)size.y != context.viewportTexture.texture.height))
    {
        UnloadRenderTexture(context.viewportTexture);
        context.viewportTexture = LoadRenderTexture((int)size.x, (int)size.y);
        context.viewportReady = (context.viewportTexture.texture.id != 0);
    }

    context.viewportHovered = ImGui::IsWindowHovered();

    RenderSceneViewport(context);

    if (context.viewportReady)
    {
        rlImGuiImageRenderTextureFit(&context.viewportTexture, false);

        if (context.viewportHovered && !context.previewSceneCamera)
        {
            Ray ray;
            ImVec2 imageMin;
            ImVec2 imageMax;

            if (GetViewportMouseRay(context, ray, imageMin, imageMax))
            {
                if (context.gridPainterEnabled && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                {
                    int gridX = 0;
                    int gridY = 0;
                    int gridZ = 0;

                    if (GetGridCellFromRay(context, ray, gridX, gridY, gridZ))
                    {
                        if (context.gridPainterMode == GridPainterMode::Paint)
                        {
                            PaintCubeAt(context, gridX, gridY, gridZ);
                        }
                        else if (context.gridPainterMode == GridPainterMode::Erase)
                        {
                            EraseCubeAt(context, gridX, gridY, gridZ);
                        }
                        else if (context.gridPainterMode == GridPainterMode::Eyedropper)
                        {
                            int cubeIndex = FindCubeAt(context, gridX, gridY, gridZ);

                            if (cubeIndex >= 0)
                            {
                                CopyCubeToTemplate(context, context.scene.cubes[cubeIndex]);
                                context.selection = { SelectionType::GridCube, cubeIndex };
                            }
                        }
                    }
                }
                else if (!context.gridPainterEnabled && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                {
                    context.selection = PickViewportSelection(context, ray);
                }
                if (context.gridPainterEnabled &&
                    IsKeyDown(KEY_LEFT_CONTROL) &&
                    IsKeyPressed(KEY_Z))
                {
                    UndoGridPainterAction(context);
                }
            }
        }
    }

    ImGui::End();
}
static Vector3 GetCameraPreviewPlayerPosition(EditorContext& context)
{
    // Use the first spawn point as the fake player position for camera preview.
    // This lets us preview followPlayer mode without actually running the game.
    if (!context.scene.playerSpawns.empty())
    {
        const auto& firstSpawn = context.scene.playerSpawns.begin()->second;

        return {
            firstSpawn.x,
            firstSpawn.y,
            firstSpawn.z
        };
    }

    // Fallback if the scene has no spawn points.
    return { 0.0f, 1.0f, 0.0f };
}

static Camera3D BuildScenePreviewCamera(EditorContext& context)
{
    Camera3D camera = context.editorCamera;

    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    const SceneCameraData& cameraData = context.scene.camera;

    if (cameraData.mode == "fixed")
    {
        camera.position = {
            cameraData.positionX,
            cameraData.positionY,
            cameraData.positionZ
        };

        camera.target = {
            cameraData.targetX,
            cameraData.targetY,
            cameraData.targetZ
        };

        return camera;
    }

    // followPlayer preview
    Vector3 fakePlayerPos = GetCameraPreviewPlayerPosition(context);

    camera.target = fakePlayerPos;

    camera.position = {
        fakePlayerPos.x + cameraData.positionX,
        fakePlayerPos.y + cameraData.positionY,
        fakePlayerPos.z + cameraData.positionZ
    };

    return camera;
}
static void RenderSceneViewport(EditorContext& context)
{
    if (context.previewDirty)
    {
        context.previewScene.loadFromData(context.scene);
        context.previewDirty = false;
    }
    if (!context.viewportReady)
        return;

    // validate texture is actually loaded
    if (context.viewportTexture.texture.id == 0)
    {
        context.viewportReady = false;
        return;
    }

    if (!context.previewSceneCamera && context.viewportHovered) {
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f)
        {
            Vector3 dir = Vector3Subtract(context.editorCamera.target, context.editorCamera.position);
            float dist = Vector3Length(dir);
            dist -= wheel * 0.5f;
            if (dist < 1.0f) dist = 1.0f;
            dir = Vector3Normalize(dir);
            context.editorCamera.position = Vector3Subtract(context.editorCamera.target, Vector3Scale(dir, dist));
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)){
            Vector2 mouseDelta = GetMouseDelta();
            UpdateCameraPro(&context.editorCamera, {0,0,0}, {mouseDelta.x * 0.3f, mouseDelta.y * 0.3f}, 0);
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)){
            Vector2 mouseDelta = GetMouseDelta();
            Vector3 right = Vector3CrossProduct(Vector3Subtract(context.editorCamera.target, context.editorCamera.position), context.editorCamera.up);
            right = Vector3Normalize(right);
            Vector3 up = context.editorCamera.up;

            Vector3 movement = Vector3Add(Vector3Scale(right, -mouseDelta.x * 0.01f), Vector3Scale(up, mouseDelta.y * 0.01f));
            context.editorCamera.position = Vector3Add(context.editorCamera.position, movement);
            context.editorCamera.target = Vector3Add(context.editorCamera.target, movement);
        }

    }
    Camera3D activeCamera = context.previewSceneCamera
    ? BuildScenePreviewCamera(context)
    : context.editorCamera;
    BeginTextureMode(context.viewportTexture);
    ClearBackground(SKYBLUE);

    RenderOptions options;
    options.drawHealthBars = false;
    options.drawPlayerUI = false;

    context.renderSystem.render(
        context.previewScene,
        context.resourceManager,
        activeCamera,
        options
    );

    BeginMode3D(activeCamera);
    DrawInvisibleCubeEditorOverlays(context);
    if (context.gridPainterEnabled)
    {
        // Cubes are centered on integer coordinates.
        // Therefore cell borders are at integer + 0.5.
        float y = (float)context.gridPainterLayerY + 0.51f;

        for (int x = -20; x <= 20; x++)
        {
            float lineX = (float)x + 0.5f;

            DrawLine3D(
                { lineX, y, -20.5f },
                { lineX, y,  20.5f },
                Fade(WHITE, 0.25f)
            );
        }

        for (int z = -20; z <= 20; z++)
        {
            float lineZ = (float)z + 0.5f;

            DrawLine3D(
                { -20.5f, y, lineZ },
                {  20.5f, y, lineZ },
                Fade(WHITE, 0.25f)
            );
        }
    }
    DrawEntityEditorOverlays(context);
    DrawEditorSelectionOverlay(context);

    int i = 0;
    // spawnpoints
    for (const auto& [id, spawn] : context.scene.playerSpawns)
    {
        Vector3 pos = {
            spawn.x,
            spawn.y + 0.5f,
            spawn.z
        };

        Color color = ORANGE;

        if (context.selection.type == SelectionType::SpawnPoint &&
            context.selection.index == i)
        {
            color = YELLOW;
        }

        DrawSphere(pos, 0.25f, color);
        DrawCircle3D(pos, 0.5f, { 1, 0, 0 }, 90.0f, color);

        i++;
    }
    EndMode3D();

    EndTextureMode();
    }

void SetupEditorStyle()
{
    ImGui::StyleColorsDark();
}

static void DrawEditorSelectionOverlay(EditorContext& context)
{
    if (context.selection.type == SelectionType::GridCube)
    {
        int i = context.selection.index;

        if (i >= 0 && i < (int)context.scene.cubes.size())
        {
            auto& cube = context.scene.cubes[i];

            Vector3 pos = {
                (float)cube.position.x,
                (float)cube.position.y,
                (float)cube.position.z
            };

            DrawCubeWires(pos, 1.08f, 1.08f, 1.08f, YELLOW);
        }
    }

/*     if (context.selection.type == SelectionType::Entity)
    {
        int i = context.selection.index;

        if (i >= 0 && i < (int)context.scene.entities.size())
        {
            const auto& entity = context.scene.entities[i];

            BoundingBox box = GetEntityBounds(entity);

            Vector3 size = {
                box.max.x - box.min.x,
                box.max.y - box.min.y,
                box.max.z - box.min.z
            };

            Vector3 center = {
                (box.min.x + box.max.x) / 2.0f,
                (box.min.y + box.max.y) / 2.0f,
                (box.min.z + box.max.z) / 2.0f
            };

            DrawCubeWires(
                center,
                size.x + 0.08f,
                size.y + 0.08f,
                size.z + 0.08f,
                YELLOW
            );
        }
    } */
}
static bool GetEntityPosition(const nlohmann::json& entity, Vector3& outPos)
{
    if (!entity.contains("TransformComponent") || !entity["TransformComponent"].is_object())
        return false;

    const auto& t = entity["TransformComponent"];

    outPos = {
        t.value("x", 0.0f),
        t.value("y", 0.0f),
        t.value("z", 0.0f)
    };

    return true;
}

static BoundingBox GetEntityBounds(const nlohmann::json& entity)
{
    Vector3 pos = { 0.0f, 0.0f, 0.0f };
    GetEntityPosition(entity, pos);

    float width = 1.0f;
    float height = 1.0f;
    float depth = 1.0f;

    float offsetX = 0.0f;
    float offsetY = 0.0f;
    float offsetZ = 0.0f;

    // Prefer Collider because it represents the actual physical/editor interaction size.
    if (entity.contains("Collider") && entity["Collider"].is_object())
    {
        const auto& c = entity["Collider"];

        if (c.value("enabled", true))
        {
            width = c.value("width", 1.0f);
            height = c.value("height", 1.0f);
            depth = c.value("depth", 1.0f);

            offsetX = c.value("offsetX", 0.0f);
            offsetY = c.value("offsetY", 0.0f);
            offsetZ = c.value("offsetZ", 0.0f);
        }
    }
    // If there is no Collider, use Renderer as a visual fallback.
    else if (entity.contains("Renderer") && entity["Renderer"].is_object())
    {
        const auto& r = entity["Renderer"];

        std::string modelID = r.value("modelID", "");

        if (!modelID.empty())
        {
            float scale = r.value("scale", 1.0f);

            width = scale;
            height = scale * 2.0f;
            depth = scale;
        }
        else
        {
            width = r.value("width", 1.0f);
            height = r.value("height", 1.0f);
            depth = r.value("depth", 1.0f);
        }
    }

    // Minimum pick size so small entities are not annoying to click.
    width = std::max(width, 0.8f);
    height = std::max(height, 1.0f);
    depth = std::max(depth, 0.8f);

    Vector3 center = {
        pos.x + offsetX,
        pos.y + offsetY,
        pos.z + offsetZ
    };

    return {
        { center.x - width / 2.0f, center.y - height / 2.0f, center.z - depth / 2.0f },
        { center.x + width / 2.0f, center.y + height / 2.0f, center.z + depth / 2.0f }
    };
}

static EditorSelection PickViewportSelection(EditorContext& context, Ray ray)
{
    float closestEntityDist = FLT_MAX;
    EditorSelection entitySelection = { SelectionType::None, -1 };

    // Pick entities first so floor cubes do not steal clicks.
    for (int i = 0; i < (int)context.scene.entities.size(); i++)
    {
        const auto& entity = context.scene.entities[i];

        Vector3 pos;
        if (!GetEntityPosition(entity, pos))
            continue;

        BoundingBox box = GetEntityBounds(entity);
        RayCollision hit = GetRayCollisionBox(ray, box);

        if (hit.hit && hit.distance < closestEntityDist)
        {
            closestEntityDist = hit.distance;
            entitySelection = { SelectionType::Entity, i };
        }
    }

    if (entitySelection.type != SelectionType::None)
        return entitySelection;

    float closestCubeDist = FLT_MAX;
    EditorSelection cubeSelection = { SelectionType::None, -1 };

    for (int i = 0; i < (int)context.scene.cubes.size(); i++)
    {
        const auto& cube = context.scene.cubes[i];

        Vector3 pos = {
            (float)cube.position.x,
            (float)cube.position.y,
            (float)cube.position.z
        };

        BoundingBox box = {
            Vector3SubtractValue(pos, 0.5f),
            Vector3AddValue(pos, 0.5f)
        };

        RayCollision hit = GetRayCollisionBox(ray, box);

        if (hit.hit && hit.distance < closestCubeDist)
        {
            closestCubeDist = hit.distance;
            cubeSelection = { SelectionType::GridCube, i };
        }
    }

    return cubeSelection;
}

static void DrawEntityEditorOverlays(EditorContext& context)
{
    for (int i = 0; i < (int)context.scene.entities.size(); i++)
    {
        const auto& entity = context.scene.entities[i];

        Vector3 pos;
        if (!GetEntityPosition(entity, pos))
            continue;

        bool selected =
            context.selection.type == SelectionType::Entity &&
            context.selection.index == i;

        // Collider box.
        if (entity.contains("Collider") && entity["Collider"].is_object())
        {
            const auto& c = entity["Collider"];

            if (c.value("enabled", true))
            {
                float width = c.value("width", 1.0f);
                float height = c.value("height", 1.0f);
                float depth = c.value("depth", 1.0f);

                Vector3 colliderCenter = {
                    pos.x + c.value("offsetX", 0.0f),
                    pos.y + c.value("offsetY", 0.0f),
                    pos.z + c.value("offsetZ", 0.0f)
                };

                Color colliderColor = c.value("isTrigger", false) ? SKYBLUE : GREEN;

                if (selected)
                    colliderColor = YELLOW;

                DrawCubeWires(colliderCenter, width, height, depth, colliderColor);
            }
        }

        // Interaction radius.
        if (entity.contains("Interactable") && entity["Interactable"].is_object())
        {
            const auto& interactable = entity["Interactable"];
            float radius = interactable.value("interactionRadius", 1.0f);

            Vector3 circlePos = {
                pos.x,
                pos.y + 0.03f,
                pos.z
            };

            Color radiusColor = selected ? ORANGE : Fade(ORANGE, 0.65f);

            DrawCircle3D(
                circlePos,
                radius,
                { 1.0f, 0.0f, 0.0f },
                90.0f,
                radiusColor
            );
        }
    }
}
