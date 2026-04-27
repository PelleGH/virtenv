cmake_minimum_required(VERSION 3.20)
project(VirtenEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 CONFIG REQUIRED)

add_executable(VirtenEditor
    src/main.cpp
    src/editor/EditorApp.cpp
    src/editor/gui/EditorUI.cpp
    src/engine/core/Engine.cpp

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

if (WIN32)
    target_link_libraries(VirtenEditor PRIVATE opengl32)
endif()