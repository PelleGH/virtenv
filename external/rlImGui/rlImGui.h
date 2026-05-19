#pragma once

#include "raylib.h"
#include "imgui.h"

void rlImGuiSetup(bool darkTheme);
void rlImGuiBegin();
void rlImGuiEnd();
void rlImGuiShutdown();

// Render texture helper - draws RenderTexture2D into an ImGui window, flipping Y
void rlImGuiImageRenderTextureFit(const RenderTexture2D* image, bool center);
