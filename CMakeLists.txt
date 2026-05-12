cmake_minimum_required(VERSION 3.20)
project(VirtenEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 CONFIG REQUIRED)
find_package(raylib CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(VirtenEditor
    src/main.cpp
    src/editor/EditorApp.cpp
    src/editor/gui/EditorUI.cpp
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
    
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp




)

target_include_directories(VirtenEditor PRIVATE
    src
    src/editor/gui
    external/imgui
    external/imgui/backends
)

target_link_libraries(VirtenEditor PRIVATE glfw)
target_link_libraries(VirtenEditor PRIVATE raylib)
target_link_libraries(VirtenEditor PRIVATE nlohmann_json::nlohmann_json)

if (WIN32)
    target_link_libraries(VirtenEditor PRIVATE opengl32)
endif()