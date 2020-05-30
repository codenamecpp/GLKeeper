#pragma once

#include "RenderProgram.h"
#include "GpuProgram.h"

// render program intended for gui draw
class GuiRenderProgram: public RenderProgram
{
public:
    GuiRenderProgram(): RenderProgram("shaders/gui.glsl")
    {
    }

private:
    void HandleProgramLoad() override
    {
        // configure input layout
        mGpuProgram->BindAttribute(eVertexAttribute_Position0, "in_pos");
        mGpuProgram->BindAttribute(eVertexAttribute_Color0, "in_color");
        mGpuProgram->BindAttribute(eVertexAttribute_Texcoord0, "in_texcoord");
    }

    void HandleProgramFree() override
    {
    }
};

