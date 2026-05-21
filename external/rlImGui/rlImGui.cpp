#include "rlImGui.h"

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "imgui.h"

#include <stdint.h>
#include <math.h>

// -----------------------------------------------------------------------
// Internal state
// -----------------------------------------------------------------------
static ImGuiContext* g_imguiContext = nullptr;
static Texture2D     g_fontTexture  = {};

// -----------------------------------------------------------------------
// Clipboard helpers (raylib 5.x)
// -----------------------------------------------------------------------
static const char* rlImGui_GetClipboard(void*)
{
    return GetClipboardText();
}

static void rlImGui_SetClipboard(void*, const char* text)
{
    SetClipboardText(text);
}

// -----------------------------------------------------------------------
// Font atlas upload
// -----------------------------------------------------------------------
static void UploadFontTexture()
{
    ImGuiIO& io = ImGui::GetIO();

    unsigned char* pixels = nullptr;
    int width = 0, height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    Image image = {};
    image.data    = pixels;
    image.width   = width;
    image.height  = height;
    image.mipmaps = 1;
    image.format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    g_fontTexture = LoadTextureFromImage(image);
    SetTextureFilter(g_fontTexture, TEXTURE_FILTER_BILINEAR);

    io.Fonts->SetTexID((ImTextureID)(uintptr_t)g_fontTexture.id);
}

// -----------------------------------------------------------------------
// Setup / Shutdown
// -----------------------------------------------------------------------
void rlImGuiSetup(bool darkTheme)
{
    g_imguiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(g_imguiContext);

    ImGuiIO& io = ImGui::GetIO();
    io.BackendPlatformName = "raylib";
    io.BackendRendererName = "raylib/rlgl";

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Clipboard
    io.GetClipboardTextFn = rlImGui_GetClipboard;
    io.SetClipboardTextFn = rlImGui_SetClipboard;

    // Display size
    io.DisplaySize = ImVec2((float)GetScreenWidth(), (float)GetScreenHeight());

    if (darkTheme)
        ImGui::StyleColorsDark();
    else
        ImGui::StyleColorsLight();

    UploadFontTexture();
}

void rlImGuiShutdown()
{
    if (g_fontTexture.id != 0)
    {
        UnloadTexture(g_fontTexture);
        g_fontTexture = {};
    }

    if (g_imguiContext)
    {
        ImGui::DestroyContext(g_imguiContext);
        g_imguiContext = nullptr;
    }
}

// -----------------------------------------------------------------------
// Per-frame input mapping
// -----------------------------------------------------------------------
static void UpdateInput()
{
    ImGuiIO& io = ImGui::GetIO();

    io.DisplaySize = ImVec2((float)GetScreenWidth(), (float)GetScreenHeight());
    io.DeltaTime   = GetFrameTime();

    // Mouse
    io.AddMousePosEvent((float)GetMouseX(), (float)GetMouseY());
    io.AddMouseButtonEvent(ImGuiMouseButton_Left,   IsMouseButtonDown(MOUSE_BUTTON_LEFT));
    io.AddMouseButtonEvent(ImGuiMouseButton_Right,  IsMouseButtonDown(MOUSE_BUTTON_RIGHT));
    io.AddMouseButtonEvent(ImGuiMouseButton_Middle, IsMouseButtonDown(MOUSE_BUTTON_MIDDLE));

    Vector2 wheel = GetMouseWheelMoveV();
    io.AddMouseWheelEvent(wheel.x, wheel.y);

    // Keyboard modifier keys
    io.AddKeyEvent(ImGuiMod_Ctrl,  IsKeyDown(KEY_LEFT_CONTROL)  || IsKeyDown(KEY_RIGHT_CONTROL));
    io.AddKeyEvent(ImGuiMod_Shift, IsKeyDown(KEY_LEFT_SHIFT)    || IsKeyDown(KEY_RIGHT_SHIFT));
    io.AddKeyEvent(ImGuiMod_Alt,   IsKeyDown(KEY_LEFT_ALT)      || IsKeyDown(KEY_RIGHT_ALT));
    io.AddKeyEvent(ImGuiMod_Super, IsKeyDown(KEY_LEFT_SUPER)    || IsKeyDown(KEY_RIGHT_SUPER));

    // Key mapping table: raylib key -> ImGuiKey
    static const struct { KeyboardKey rl; ImGuiKey ig; } keymap[] =
    {
        { KEY_TAB,            ImGuiKey_Tab },
        { KEY_LEFT,           ImGuiKey_LeftArrow },
        { KEY_RIGHT,          ImGuiKey_RightArrow },
        { KEY_UP,             ImGuiKey_UpArrow },
        { KEY_DOWN,           ImGuiKey_DownArrow },
        { KEY_PAGE_UP,        ImGuiKey_PageUp },
        { KEY_PAGE_DOWN,      ImGuiKey_PageDown },
        { KEY_HOME,           ImGuiKey_Home },
        { KEY_END,            ImGuiKey_End },
        { KEY_INSERT,         ImGuiKey_Insert },
        { KEY_DELETE,         ImGuiKey_Delete },
        { KEY_BACKSPACE,      ImGuiKey_Backspace },
        { KEY_SPACE,          ImGuiKey_Space },
        { KEY_ENTER,          ImGuiKey_Enter },
        { KEY_ESCAPE,         ImGuiKey_Escape },
        { KEY_LEFT_CONTROL,   ImGuiKey_LeftCtrl },
        { KEY_LEFT_SHIFT,     ImGuiKey_LeftShift },
        { KEY_LEFT_ALT,       ImGuiKey_LeftAlt },
        { KEY_LEFT_SUPER,     ImGuiKey_LeftSuper },
        { KEY_RIGHT_CONTROL,  ImGuiKey_RightCtrl },
        { KEY_RIGHT_SHIFT,    ImGuiKey_RightShift },
        { KEY_RIGHT_ALT,      ImGuiKey_RightAlt },
        { KEY_RIGHT_SUPER,    ImGuiKey_RightSuper },
        { KEY_CAPS_LOCK,      ImGuiKey_CapsLock },
        { KEY_F1,  ImGuiKey_F1  }, { KEY_F2,  ImGuiKey_F2  }, { KEY_F3,  ImGuiKey_F3  },
        { KEY_F4,  ImGuiKey_F4  }, { KEY_F5,  ImGuiKey_F5  }, { KEY_F6,  ImGuiKey_F6  },
        { KEY_F7,  ImGuiKey_F7  }, { KEY_F8,  ImGuiKey_F8  }, { KEY_F9,  ImGuiKey_F9  },
        { KEY_F10, ImGuiKey_F10 }, { KEY_F11, ImGuiKey_F11 }, { KEY_F12, ImGuiKey_F12 },
        { KEY_A, ImGuiKey_A }, { KEY_B, ImGuiKey_B }, { KEY_C, ImGuiKey_C },
        { KEY_D, ImGuiKey_D }, { KEY_E, ImGuiKey_E }, { KEY_F, ImGuiKey_F },
        { KEY_G, ImGuiKey_G }, { KEY_H, ImGuiKey_H }, { KEY_I, ImGuiKey_I },
        { KEY_J, ImGuiKey_J }, { KEY_K, ImGuiKey_K }, { KEY_L, ImGuiKey_L },
        { KEY_M, ImGuiKey_M }, { KEY_N, ImGuiKey_N }, { KEY_O, ImGuiKey_O },
        { KEY_P, ImGuiKey_P }, { KEY_Q, ImGuiKey_Q }, { KEY_R, ImGuiKey_R },
        { KEY_S, ImGuiKey_S }, { KEY_T, ImGuiKey_T }, { KEY_U, ImGuiKey_U },
        { KEY_V, ImGuiKey_V }, { KEY_W, ImGuiKey_W }, { KEY_X, ImGuiKey_X },
        { KEY_Y, ImGuiKey_Y }, { KEY_Z, ImGuiKey_Z },
        { KEY_ZERO,  ImGuiKey_0 }, { KEY_ONE,   ImGuiKey_1 }, { KEY_TWO,   ImGuiKey_2 },
        { KEY_THREE, ImGuiKey_3 }, { KEY_FOUR,  ImGuiKey_4 }, { KEY_FIVE,  ImGuiKey_5 },
        { KEY_SIX,   ImGuiKey_6 }, { KEY_SEVEN, ImGuiKey_7 }, { KEY_EIGHT, ImGuiKey_8 },
        { KEY_NINE,  ImGuiKey_9 },
    };

    for (auto& k : keymap)
    {
        if (IsKeyPressed(k.rl) || IsKeyReleased(k.rl))
            io.AddKeyEvent(k.ig, IsKeyDown(k.rl));
    }

    // Text input
    int ch = 0;
    while ((ch = GetCharPressed()) != 0)
        io.AddInputCharacter((unsigned int)ch);
}

// -----------------------------------------------------------------------
// Rendering
// -----------------------------------------------------------------------
static void RenderDrawData(ImDrawData* drawData)
{
    rlDisableBackfaceCulling();
    rlDisableDepthTest();

    for (int i = 0; i < drawData->CmdListsCount; i++)
    {
        const ImDrawList* cmdList = drawData->CmdLists[i];

        for (int j = 0; j < cmdList->CmdBuffer.Size; j++)
        {
            const ImDrawCmd& cmd = cmdList->CmdBuffer[j];

            if (cmd.UserCallback)
            {
                cmd.UserCallback(cmdList, &cmd);
                continue;
            }

            // Scissor
            ImVec2 pos  = drawData->DisplayPos;
            int sx = (int)(cmd.ClipRect.x - pos.x);
            int sy = (int)(cmd.ClipRect.y - pos.y);
            int sw = (int)(cmd.ClipRect.z - cmd.ClipRect.x);
            int sh = (int)(cmd.ClipRect.w - cmd.ClipRect.y);
            BeginScissorMode(sx, sy, sw, sh);

            unsigned int textureId = (unsigned int)(uintptr_t)cmd.GetTexID();

            rlBegin(RL_TRIANGLES);
            rlSetTexture(textureId);

            for (unsigned int k = 0; k < cmd.ElemCount; k++)
            {
                unsigned int idx = cmdList->IdxBuffer[cmd.IdxOffset + k];
                const ImDrawVert& v = cmdList->VtxBuffer[cmd.VtxOffset + idx];

                rlColor4ub(
                    (v.col >>  0) & 0xFF,
                    (v.col >>  8) & 0xFF,
                    (v.col >> 16) & 0xFF,
                    (v.col >> 24) & 0xFF
                );
                rlTexCoord2f(v.uv.x, v.uv.y);
                rlVertex2f(v.pos.x, v.pos.y);
            }

            rlEnd();
            rlSetTexture(0);
        }
    }

    EndScissorMode();
    rlEnableBackfaceCulling();
    rlEnableDepthTest();
}

// -----------------------------------------------------------------------
// Public frame API
// -----------------------------------------------------------------------
void rlImGuiBegin()
{
    UpdateInput();
    ImGui::NewFrame();
}

void rlImGuiEnd()
{
    ImGui::Render();
    RenderDrawData(ImGui::GetDrawData());
}

// -----------------------------------------------------------------------
// Render texture helper
// -----------------------------------------------------------------------
void rlImGuiImageRenderTextureFit(const RenderTexture2D* image, bool center)
{
    ImVec2 area = ImGui::GetContentRegionAvail();

    float scale = (area.x / (float)image->texture.width);
    float y     = (float)image->texture.height * scale;

    if (y > area.y)
    {
        scale = area.y / (float)image->texture.height;
    }

    float w = (float)image->texture.width  * scale;
    float h = (float)image->texture.height * scale;

    if (center)
    {
        ImGui::SetCursorPosX((area.x - w) * 0.5f + ImGui::GetCursorPosX());
        ImGui::SetCursorPosY((area.y - h) * 0.5f + ImGui::GetCursorPosY());
    }

    // Flip Y (raylib render textures are upside down relative to ImGui)
    ImGui::Image(
        (ImTextureID)(uintptr_t)image->texture.id,
        ImVec2(w, h),
        ImVec2(0, 1),
        ImVec2(1, 0)
    );
}