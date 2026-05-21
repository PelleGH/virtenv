#pragma once

#include "../EditorContext.h"
#include <string>

void DrawAssets(EditorContext& context);
void DrawViewport(EditorContext& context);
void DrawHierarchy(EditorContext& context);
void DrawInspector(EditorContext& context);

void LoadEditorScene(EditorContext& context, int index);
void MarkSceneChanged(EditorContext& context);
void AddCube(EditorContext& context, const std::string& type);
void AddSpawnPoint(EditorContext& context);