#pragma once

struct GLFWwindow;

class EditorApp
{
public:
    bool init();
    void run();
    void shutdown();

private:
    GLFWwindow* window = nullptr;
    const char* glsl_version = "#version 130";

    void beginFrame();
    void endFrame();
};