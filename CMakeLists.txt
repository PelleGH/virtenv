cmake_minimum_required(VERSION 3.20)
project(VirtenEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(raylib CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(VirtenEditor
    src/main.cpp
    src/editor/EditorApp.cpp
    src/editor/gui/EditorUI.cpp
    src/editor/gui/EditorHelpers.cpp
    src/editor/gui/pages/EditorAssets.cpp
    src/editor/gui/pages/EditorHierarchy.cpp
    src/editor/gui/pages/EditorInspector.cpp
    src/editor/gui/pages/EditorViewport.cpp
    src/editor/gui/pages/ProjectLauncherPanel.cpp
    src/editor/gui/pages/EditorGridPainter.cpp
    src/editor/gui/pages/EditorSceneSettingsPanel.cpp
    src/editor/project/ProjectManager.cpp
    
    src/engine/core/Engine.cpp
    src/engine/scene/Scene.cpp
    src/engine/scene/SceneLoader.cpp
    src/engine/scene/SceneManager.cpp
    src/engine/systems/RenderSystem.cpp
    src/engine/systems/InputSystem.cpp
    src/engine/systems/MovementSystem.cpp
    src/engine/systems/CollisionSystem.cpp
    src/engine/systems/CameraSystem.cpp
    src/engine/systems/TriggerSystem.cpp
    src/engine/systems/SpawnSystem.cpp
    src/engine/systems/ConditionSystem.cpp
    src/engine/systems/DialogueManager.cpp
    src/engine/systems/CombatSystem.cpp
    src/engine/systems/AISystem.cpp
    src/engine/systems/ProjectileSystem.cpp
    src/engine/managers/QuestManager.cpp
    src/engine/ecs/EntityManager.cpp
    src/engine/ecs/EntityFactory.cpp
    src/engine/resource/ResourceManager.cpp
    src/engine/systems/InteractionSystem.cpp
    src/engine/systems/InventorySystem.cpp
    src/engine/systems/InventoryUISystem.cpp

    
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/rlImGui/rlImGui.cpp




)

target_include_directories(VirtenEditor PRIVATE
    src
    src/editor/gui
    external/imgui
    external/imgui/backends
    external/rlImGui
)
target_compile_definitions(VirtenEditor PRIVATE NO_FONT_AWESOME)

target_link_libraries(VirtenEditor PRIVATE raylib)
target_link_libraries(VirtenEditor PRIVATE nlohmann_json::nlohmann_json)

#game executable

add_executable(Runtime
    src/RuntimeMain.cpp
    
    src/engine/core/Engine.cpp
    src/engine/scene/Scene.cpp
    src/engine/scene/SceneLoader.cpp
    src/engine/scene/SceneManager.cpp
    src/engine/systems/RenderSystem.cpp
    src/engine/systems/InputSystem.cpp
    src/engine/systems/MovementSystem.cpp
    src/engine/systems/CollisionSystem.cpp
    src/engine/systems/CameraSystem.cpp
    src/engine/systems/TriggerSystem.cpp
    src/engine/systems/SpawnSystem.cpp
    src/engine/systems/ConditionSystem.cpp
    src/engine/systems/DialogueManager.cpp
    src/engine/systems/CombatSystem.cpp
    src/engine/systems/AISystem.cpp
    src/engine/systems/ProjectileSystem.cpp
    src/engine/managers/QuestManager.cpp
    src/engine/ecs/EntityManager.cpp
    src/engine/ecs/EntityFactory.cpp
    src/engine/resource/ResourceManager.cpp
    src/engine/systems/InteractionSystem.cpp
    src/engine/systems/InventorySystem.cpp
    src/engine/systems/InventoryUISystem.cpp
)

# The runtime only needs the src directory
target_include_directories(Runtime PRIVATE src)

# Link required libraries
target_link_libraries(Runtime PRIVATE raylib)
target_link_libraries(Runtime PRIVATE nlohmann_json::nlohmann_json)