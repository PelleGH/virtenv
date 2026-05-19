#include "ProjectLauncherPanel.h"

void DrawProjectLauncher(EditorContext& context) {
    if (context.availableProjects.empty()) {
        RefreshProjectList(context);
    }

    ImGui::Begin("Project Launcher");
    
    ImGui::Text("Active Project: %s", context.projectName.c_str());
    if (ImGui::Button("Refresh Directory List")) {
        RefreshProjectList(context);
    }
    
    ImGui::Separator();
    
    ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Create New Game Project:");
    
    static char folderNameBuf[64] = "";
    static char projNameBuf[128] = "";

    ImGui::InputText("Folder Name (e.g. MyRPG)", folderNameBuf, IM_ARRAYSIZE(folderNameBuf));
    ImGui::InputText("Display Game Name", projNameBuf, IM_ARRAYSIZE(projNameBuf));

    if (ImGui::Button("Generate Project Templates")) {
        // Run our automated file generation routine!
        CreateNewProject(context, folderNameBuf, projNameBuf);
        
        // Clear out the text inputs so they are ready for next time
        std::memset(folderNameBuf, 0, sizeof(folderNameBuf));
        std::memset(projNameBuf, 0, sizeof(projNameBuf));
    }
    // --------------------------------------------------

    ImGui::Separator();
    ImGui::Text("Available Game Projects:");

    for (const auto& projName : context.availableProjects) {
        ImGui::BulletText("%s", projName.c_str());
        ImGui::SameLine(ImGui::GetWindowWidth() - 80);
        
        char buttonLabel[64];
        snprintf(buttonLabel, sizeof(buttonLabel), "Open##%s", projName.c_str());
        
        if (ImGui::Button(buttonLabel)) {
            LoadProject(context, projName); 
        }
    }

    ImGui::End();
}