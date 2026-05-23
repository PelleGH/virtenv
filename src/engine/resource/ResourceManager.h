#pragma once
#include <raylib.h>
#include <string>
#include <unordered_map>
#include <iostream>
#include <string>
#include "../ecs/Components.h"

class ResourceManager {
    public: 
        void LoadTexture2D(const std::string& id, const std::string& filepath);
        void LoadModel3D(const std::string& id, const std::string& filepath);

        void AddModel(const std::string& id, Model model);

        Texture2D GetTexture(const std::string& id);
        Model GetModel(const std::string& id);

        bool hasModel(const std::string& id);
        bool hasTexture(const std::string& id);

        void LoadFromManifest(const std::string& path);

        void clear();

        ItemData getItem(const std::string& id);
        bool loadItems(const std::string& path);

        // Getters for lists
        const std::unordered_map<std::string, Texture2D>& GetAllTextures() const { return textures; }
        const std::unordered_map<std::string, Model>& GetAllModels() const { return models; }
        const std::unordered_map<std::string, ItemData>& GetAllItems() const { return itemDatabase; }

    private:
        std::unordered_map<std::string, Texture2D> textures;
        std::unordered_map<std::string, Model> models;
        std::unordered_map<std::string, ItemData> itemDatabase;

};
