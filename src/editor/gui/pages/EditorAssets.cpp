#include "../EditorPanels.h"
#include "imgui.h"

void DrawAssets(EditorContext& context)
{
    // In order to know which project specific assets window belong to
    std::string assetsTitle = "Assets";

    if (!context.projectName.empty()) {
        assetsTitle = assetsTitle + " (" + context.projectName + ")";
    }

    // ### acts like a hidden ID for ImGUi to remember that the assets window is the same,
    // even when the name changes
    assetsTitle = assetsTitle + "###AssetsWindow";

    ImGui::Begin(assetsTitle.c_str(), NULL, ImGuiWindowFlags_NoFocusOnAppearing);

    if (ImGui::TreeNode("Textures")) {

        // Loop through all textures in resource
        for (const auto& [id, tex] : context.resourceManager.GetAllTextures()) {

            // If current id equals editors marker / pointer
           bool isSelected = (context.selection.type == SelectionType::AssetTexture && context.selection.assetId == id);

           // Write the row and update editors context when clicked
           if (ImGui::Selectable(id.c_str(), isSelected)) {
                context.selection.type = SelectionType::AssetTexture;
                context.selection.assetId = id;
           }
        }
        ImGui::TreePop(); 
    }

    if (ImGui::TreeNode("Models")) {
        
        // Loop through all models in resource
        for (const auto& [id, model] : context.resourceManager.GetAllModels()) {

            // If current id equals editors marker / pointer
            bool isSelected = (context.selection.type == SelectionType::AssetModel && context.selection.assetId == id);
            
            // Write the row and update editors context when clicked
            if (ImGui::Selectable(id.c_str(), isSelected)) {
                context.selection.type = SelectionType::AssetModel;
                context.selection.assetId = id;
            }
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Items")) {
        
        for (const auto& [id, item] : context.resourceManager.GetAllItems()) {

            char label[128];
            snprintf(label, sizeof(label), "%s (%s)", id.c_str(), item.name.c_str());

            // If current id equals editors marker / pointer
            bool isSelected = (context.selection.type == SelectionType::AssetItem && context.selection.assetId == id);
            
            // Write the row and update editors context when clicked, use the created label instead of ID
            if (ImGui::Selectable(label, isSelected)) {
                context.selection.type = SelectionType::AssetItem;
                context.selection.assetId = id;
            }
        }
        ImGui::TreePop();
    }
    ImGui::End();
}