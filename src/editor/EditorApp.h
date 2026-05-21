#pragma once
#include "editor/EditorContext.h"

class EditorApp
{
public:
    bool init();
    void run();
    void shutdown();

private:
    EditorContext context;
};