#pragma once

#include "editor/EditorContext.h"
#include <string>

void LoadProject(EditorContext& context, const std::string& chosenProjectFolder);
void SaveProject(EditorContext& context);
void RefreshProjectList(EditorContext& context);
void CreateNewProject(EditorContext& context, const std::string& folderName, const std::string& userProjectName);
void LoadLastProject(EditorContext& context);
