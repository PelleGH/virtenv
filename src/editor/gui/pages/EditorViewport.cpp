#include "../EditorPanels.h"
#include "imgui.h"
#include "raylib.h"
#include "rlImGui.h"
#include "raymath.h"

#include <cfloat>

static void RenderSceneViewport(EditorContext& context);
static void DrawEditorSelectionOverlay(EditorContext& context);

void DrawViewport(EditorContext& context)
{
    ImGui::Begin("Viewport");

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

        if (context.viewportHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            ImVec2 imageMin = ImGui::GetItemRectMin();
            ImVec2 imageMax = ImGui::GetItemRectMax();

            Vector2 mousePos = GetMousePosition();

            Vector2 relativePos = {
                mousePos.x - imageMin.x,
                mousePos.y - imageMin.y
            };

            float imageWidth = imageMax.x - imageMin.x;
            float imageHeight = imageMax.y - imageMin.y;

            if (relativePos.x >= 0 && relativePos.y >= 0 &&
                relativePos.x <= imageWidth && relativePos.y <= imageHeight)
            {
                Vector2 textureMouse = {
                    relativePos.x * context.viewportTexture.texture.width / imageWidth,
                    relativePos.y * context.viewportTexture.texture.height / imageHeight
                };

                Ray ray = GetScreenToWorldRayEx(
                    textureMouse,
                    context.editorCamera,
                    context.viewportTexture.texture.width,
                    context.viewportTexture.texture.height
                );

                float closestDist = FLT_MAX;
                EditorSelection newSelection = { SelectionType::None, -1 };

                for (int i = 0; i < (int)context.scene.cubes.size(); i++)
                {
                    auto& cube = context.scene.cubes[i];

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

                    if (hit.hit && hit.distance < closestDist)
                    {
                        closestDist = hit.distance;
                        newSelection = { SelectionType::GridCube, i };
                    }
                }

                for (int i = 0; i < (int)context.scene.entities.size(); i++)
                {
                    auto& entity = context.scene.entities[i];
                    if (!entity.contains("TransformComponent"))
                        continue;

                    auto& t = entity["TransformComponent"];

                    Vector3 pos = {
                        t.value("x", 0.0f),
                        t.value("y", 0.0f),
                        t.value("z", 0.0f)
                    };

                    float width = t.value("width", 1.0f);
                    float height = t.value("height", 1.0f);
                    float depth = t.value("depth", 1.0f);

                    BoundingBox box = {
                        {
                            pos.x - width / 2.0f,
                            pos.y - height / 2.0f,
                            pos.z - depth / 2.0f
                        },
                        {
                            pos.x + width / 2.0f,
                            pos.y + height / 2.0f,
                            pos.z + depth / 2.0f
                        }
                    };

                    RayCollision hit = GetRayCollisionBox(ray, box);

                    if (hit.hit && hit.distance < closestDist)
                    {
                        closestDist = hit.distance;
                        newSelection = { SelectionType::Entity, i };
                    }
                }

                context.selection = newSelection;
            }
        }
    }

    ImGui::End();
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

    if (context.viewportHovered) {
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
    BeginTextureMode(context.viewportTexture);
    ClearBackground(SKYBLUE);

    RenderOptions options;
    options.drawHealthBars = false;
    options.drawPlayerUI = false;

    context.renderSystem.render(
        context.previewScene,
        context.resourceManager,
        context.editorCamera,
        options
    );

    BeginMode3D(context.editorCamera);
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
}