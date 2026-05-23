#pragma once

#include "../EditorContext.h"
#include <string>

void DrawAssets(EditorContext& context);
void DrawViewport(EditorContext& context);
void DrawHierarchy(EditorContext& context);
void DrawInspector(EditorContext& context);
void LoadComponentSchemas(EditorContext& context);

void LoadEditorScene(EditorContext& context, int index);
void MarkSceneChanged(EditorContext& context);
void AddCube(EditorContext& context, const std::string& type);
void AddSpawnPoint(EditorContext& context);
void AddEntity(EditorContext& context, const std::string& type);
void RequestLoadEditorScene(EditorContext& context, int index);
void DrawUnsavedScenePopup(EditorContext& context);
void SaveCurrentEditorScene(EditorContext& context);

bool RequestEditorExit(EditorContext& context);
void DrawUnsavedExitPopup(EditorContext& context);
// grid painter helpers
void DrawGridPainter(EditorContext& context);

GridCube MakeCubeFromTemplate(
    const CubeTemplate& cubeTemplate,
    int x,
    int y,
    int z
);

int FindCubeAt(EditorContext& context, int x, int y, int z);
void PaintCubeAt(EditorContext& context, int x, int y, int z);
void EraseCubeAt(EditorContext& context, int x, int y, int z);
void CopyCubeToTemplate(EditorContext& context, const GridCube& cube);
void UndoGridPainterAction(EditorContext& context);