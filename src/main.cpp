#include "editor/EditorApp.h"

int main()
{
    EditorApp app;

    if (!app.init())
        return 1;

    app.run();
    app.shutdown();

    return 0;
}