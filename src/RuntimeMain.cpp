#include "engine/core/Engine.h"
#include <iostream>

int main(int argc, char* argv[])
{
    Engine engine;
    
    // We pass "./" so the engine knows to ignore EditorSettings.json 
    // and just load the project data sitting right next to it!
    if (engine.init("./")) 
    {
        engine.run();
    }
    else
    {
        std::cerr << "Failed to initialize game. Missing project files?\n";
    }
    
    engine.shutdown();
    return 0;
}