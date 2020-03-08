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

    void SetViewProjectionMatrix(const glm::mat4& viewProjectionMatrix)
    {
        debug_assert(IsProgramLoaded());

        if (mUniformID_view_projection_matrix == GpuVariable_NULL)
        {
            debug_assert(false);
            return;
        }
        mGpuProgram->SetUniformParam(mUniformID_view_projection_matrix, viewProjectionMatrix);
    }

private:
    void OnProgramLoad() override
    {
        mUniformID_view_projection_matrix = mGpuProgram->QueryUniformLocation("view_projection_matrix");
        debug_assert(mUniformID_view_projection_matrix != GpuVariable_NULL);
    }

    void OnProgramFree() override
    {
        mUniformID_view_projection_matrix = GpuVariable_NULL;
    }

private:
    // render constants
    GpuVariableLocation mUniformID_view_projection_matrix = GpuVariable_NULL;
};
