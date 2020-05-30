#pragma once

#include "RenderProgram.h"
#include "GpuProgram.h"

// render program intended for debug draw
class DebugDrawRenderProgram: public RenderProgram
{
public:
    DebugDrawRenderProgram(): RenderProgram("shaders/debug.glsl")
    {
    }

private:
    void HandleProgramLoad() override
    {
        // configure input layout
        mGpuProgram->BindAttribute(eVertexAttribute_Position0, "in_pos");
        mGpuProgram->BindAttribute(eVertexAttribute_Color0, "in_color");
    }

    void HandleProgramFree() override
    {
    }
};
