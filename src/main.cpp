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
    
    

    //if (!editor.init())
    //    return 1;

    //engine.run();

    //engine.shutdown();
    
    

    if (!editor.init())
       return 1;

    editor.run(engine.getSceneManager());

    editor.shutdown();

    return 0;
}