#pragma once

#include <string>
#include "SceneData.h"

class SceneLoader
{
public:
    static bool loadFromFile(const std::string& path, SceneData& outData);
};