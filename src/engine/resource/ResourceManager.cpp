#include "ResourceManager.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

void ResourceManager::LoadTexture2D(const std::string& id, const std::string& filepath){
    if (textures.find(id) == textures.end()) {
        textures[id] = LoadTexture(filepath.c_str());
        std::cout << "Loaded texture: " << id << '\n';
    }
}

void ResourceManager::LoadModel3D(const std::string& id, const std::string& filepath) {
    if (models.find(id) == models.end()) {
        models[id] = LoadModel(filepath.c_str());
        std::cout << "Loaded 3D model: " << id << '\n';
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
    std::ifstream file(path);
    
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
}
