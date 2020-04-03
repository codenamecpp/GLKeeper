#pragma once

#include "UIRenderProgram.h"

class UIRenderer: public cxx::noncopyable
{
public:

    // setup ui renderer internal resources
    // @returns false on error
    bool Initialize();
    void Deinit();
    
    void RenderFrameBegin();
    void RenderFrameEnd();

private:
    UIRenderProgram mUIRenderProgram;
    glm::mat4 mTransformationMatrix2D;
};