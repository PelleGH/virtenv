#include "ResourceManager.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

void ResourceManager::SetProjectPath(const std::string& projectName){
    currentProjectPath = "Projects/" + projectName + "/assets/";
}

std::string ResourceManager::GetAssetPath(const std::string& localPath) const{
    return currentProjectPath + localPath;
}

void ResourceManager::LoadTexture2D(const std::string& id, const std::string& filepath){
    if (textures.find(id) == textures.end()) {

        std::string fullPath = GetAssetPath(filepath);

        textures[id] = LoadTexture(fullPath.c_str());
        std::cout << "Loaded texture: " << id << " from: " << fullPath << '\n';
    }
}

void ResourceManager::LoadModel3D(const std::string& id, const std::string& filepath) {
    if (models.find(id) == models.end()) {

        std::string fullPath = GetAssetPath(filepath);

        models[id] = LoadModel(fullPath.c_str());
        std::cout << "Loaded 3D model: " << id << " from: " << fullPath << '\n';
    }
}

void ResourceManager::AddModel(const std::string& id, Model model){
    auto findID = models.find(id);

    // Checks wether the ID already exist, throw away the old ID
    if (findID != models.end())
    {
        UnloadModel(findID -> second);
    }
    
    models[id] = model;
}

Texture2D ResourceManager::GetTexture(const std::string& id){
    return textures[id];
}

Model ResourceManager::GetModel(const std::string& id){
    return models[id];
}

bool ResourceManager::hasTexture(const std::string& id) { 
    if (textures.count(id) > 0){
        return true;
    }
    return false;
}

bool ResourceManager::hasModel(const std::string& id) { 
    if (models.count(id) > 0){
        return true;
    }
    return false;
}

// Opens "assets.json" and builds at start
void ResourceManager::LoadFromManifest(const std::string& path){

    std::string fullPath = GetAssetPath(path); 
    
    std::ifstream file(fullPath);
    
    if (!file.is_open())
    {
        std::cerr << "ERROR: Could not find manifest file: " << path << std::endl;
        return;
    }
    nlohmann::json data;
    file >> data;

    // Loads all textures
    for (auto& tex : data["textures"])
    {
        LoadTexture2D(tex["id"], tex["path"]);
    }

    // Builds cubes with textures
    for (auto& m : data["cube_models"])
    {
        float s = m["size"];
        std::string modelID = m["id"];
        std::string texID = m["textureId"];

        Model model = LoadModelFromMesh(GenMeshCube(s, s, s));

        if (hasTexture(texID))
        {
            model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = GetTexture(texID);
        }

        AddModel(modelID, model);
    }

    // Load 3D-Models
    if (data.contains("3Dmodels") && data["3Dmodels"].is_array()) {
        
        for (auto& m : data["3Dmodels"]) {
            std::string modelID = m.value("id", "");
            std::string filepath = m.value("path", "");

            if (!modelID.empty() && !filepath.empty()) {
                LoadModel3D(modelID, filepath);
            } 
        }  
    } 
}

void ResourceManager::clear() {
    for (auto& [id, tex] : textures) UnloadTexture(tex);
    for (auto& [id, mod] : models) UnloadModel(mod);
    textures.clear();
    models.clear();
    itemDatabase.clear();
}

bool ResourceManager::loadItems(const std::string& path) {

    std::string fullPath = GetAssetPath(path); 

    std::ifstream file(fullPath);
    if (!file.is_open()) return false;

    nlohmann::json data;
    file >> data;

    for (const auto& itemJson : data["items"]) {
        ItemData item;
        item.id = itemJson.value("id", "");
        item.name = itemJson.value("name", "Unknown");
        
        std::string slotStr = itemJson.value("slot", "None");
        if (slotStr == "Weapon") item.slot = EquipSlot::Weapon;
        else if (slotStr == "Armor") item.slot = EquipSlot::Armor;
        else if (slotStr == "Consumable") item.slot = EquipSlot::Consumable;
        
        item.damageBonus = itemJson.value("damageBonus", 0);
        item.healthBonus = itemJson.value("healthBonus", 0);
        item.defenseBonus = itemJson.value("defenseBonus", 0);

        item.rawData = itemJson;

        itemDatabase[item.id] = item;
    }
    std::cout << "Loaded " << itemDatabase.size() << " items into database.\n";
    return true;
}

ItemData ResourceManager::getItem(const std::string& id) {
    if (itemDatabase.find(id) != itemDatabase.end()) {
        return itemDatabase[id];
    }
    return ItemData{}; // Return empty if not found
}
