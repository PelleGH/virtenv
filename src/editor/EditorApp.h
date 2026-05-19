#include "editor/EditorContext.h"

struct GLFWwindow;
class SceneManager;
class EditorApp
{
public:
    bool init();
    void run(SceneManager& sceneManager);
    void shutdown();

private:
    GLFWwindow* window = nullptr;
    const char* glsl_version = "#version 130";

    EditorContext context;

    void beginFrame();
    void endFrame();
};