#pragma once

#include "GraphicsDefs.h"
#include "GuiRenderProgram.h"

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

    // process single frame logic
    void UpdateFrame();

    // force reload all render programs
    void ReloadRenderPrograms();

private:
    // begin/end render gui geometry
    void Enter2D();
    void Leave2D();

private:
    GuiRenderProgram mGuiRenderProgram;

    glm::mat4 mTransformationMatrix2D;
};

extern RenderManager gRenderManager;