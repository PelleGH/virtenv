#include "engine/core/Engine.h"
#include "editor/EditorApp.h"

int main()
{
    Engine engine;
    EditorApp editor;
    int i = 1;

    if (i == 1){
        if (!editor.init())
            return 1;

        editor.run();

        editor.shutdown();
    }
    else {
        if (!engine.init())
            return 1;

        engine.run();

        engine.shutdown();
    }
    
    

    /*
    if (!engine.init())
        return 1;

        engine.run();

    engine.shutdown();
    */

    return 0;
}