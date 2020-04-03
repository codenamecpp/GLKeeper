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

    void SetViewProjectionMatrix(const glm::mat4& viewProjectionMatrix)
    {
        debug_assert(IsProgramLoaded());
        mGpuProgram->SetUniformParam(mUniformID_view_projection_matrix, viewProjectionMatrix);
    }

private:
    void OnProgramLoad() override
    {
        mUniformID_view_projection_matrix = mGpuProgram->QueryUniformLocation("view_projection_matrix");
        debug_assert(mUniformID_view_projection_matrix != GpuVariable_NULL);

        // configure input layout
        mGpuProgram->BindAttribute(eVertexAttribute_Position0, "in_pos");
        mGpuProgram->BindAttribute(eVertexAttribute_Color0, "in_color");
        mGpuProgram->BindAttribute(eVertexAttribute_Texcoord0, "in_texcoord");
    }

    void OnProgramFree() override
    {
        mUniformID_view_projection_matrix = GpuVariable_NULL;
    }

private:
    // render constants
    GpuVariableLocation mUniformID_view_projection_matrix = GpuVariable_NULL;
};

