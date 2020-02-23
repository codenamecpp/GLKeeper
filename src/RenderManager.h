#pragma once

#include "GraphicsDefs.h"

// master render system, it is intended to manage rendering pipeline of the game
class RenderManager: public cxx::noncopyable
{
public:

    // setup rendering system internal resources
    // @returns false on error
    bool Initialize();
    void Deinit();

    // render game frame routine
    void RenderFrame();

    // force reload all render programs
    void ReloadRenderPrograms();

};

extern RenderManager gRenderManager;