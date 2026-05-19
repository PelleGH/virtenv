#include "engine/core/Engine.h"
#include "editor/EditorApp.h"

int main()
{
    Engine engine;
    EditorApp editor;

    if (!editor.init())
        return 1;

    editor.run();

    editor.shutdown();
    
    

    /*
    if (!engine.init())
        return 1;

        engine.run();

    engine.shutdown();
    */

    return 0;
}