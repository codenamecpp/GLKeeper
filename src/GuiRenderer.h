#pragma once

#include "GuiRenderProgram.h"

class GuiRenderer: public cxx::noncopyable
{
public:
    // setup gui renderer internal resources
    // @returns false on error
    bool Initialize();
    void Deinit();
    
    void RenderFrameBegin();
    void RenderFrameEnd();

private:
    GuiRenderProgram mGuiRenderProgram;

    glm::mat4 mTransformationMatrix2D;
};