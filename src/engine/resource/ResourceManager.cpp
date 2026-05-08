#include "ResourceManager.h"

void ResourceManager::LoadTexture2D(const std::string& id, const std::string& filepath){
    if (textures.find(id) == textures.end()) {
        textures[id] = LoadTexture(filepath.c_str());
        std::cout << "Laddade textur: " << id << '\n';
    }
}

void ResourceManager::LoadModel3D(const std::string& id, const std::string& filepath) {
    if (models.find(id) == models.end()) {
        models[id] = LoadModel(filepath.c_str());
        std::cout << "Laddade 3D-modell: " << id << '\n';
    }
}

void ResourceManager::AddModel(const std::string& id, Model model){
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

void ResourceManager::clear() {
    for (auto& [id, tex] : textures) UnloadTexture(tex);
    for (auto& [id, mod] : models) UnloadModel(mod);
    textures.clear();
    models.clear();
}
