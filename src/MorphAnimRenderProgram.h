#pragma once

#include "RenderProgram.h"
#include "GpuProgram.h"

// blend frames animation shader
class MorphAnimRenderProgram: public RenderProgram
{
public:
    MorphAnimRenderProgram(): RenderProgram("shaders/anim_blend_frames.glsl")
    {
    }

    void SetViewProjectionMatrix(const glm::mat4& viewProjectionMatrix)
    {
        debug_assert(IsProgramLoaded());
        mGpuProgram->SetUniformParam(mUniformID_view_projection_matrix, viewProjectionMatrix);
    }

    void SetModelMatrix(const glm::mat4& modelMatrix)
    {
        debug_assert(IsProgramLoaded());
        mGpuProgram->SetUniformParam(mUniformID_model_matrix, modelMatrix);
    }
    
    void SetMixFrames(float mixFrames)
    {
        debug_assert(IsProgramLoaded());
        mGpuProgram->SetUniformParam(mUniformID_mix_frames, mixFrames);
    }

private:
    void OnProgramLoad() override
    {
        mUniformID_view_projection_matrix = mGpuProgram->QueryUniformLocation("view_projection_matrix");
        debug_assert(mUniformID_view_projection_matrix != GpuVariable_NULL);

        mUniformID_model_matrix = mGpuProgram->QueryUniformLocation("model_matrix");
        debug_assert(mUniformID_model_matrix != GpuVariable_NULL);

        mUniformID_mix_frames = mGpuProgram->QueryUniformLocation("mix_frames");
        debug_assert(mUniformID_mix_frames != GpuVariable_NULL);

        // configure input layout
        mGpuProgram->BindAttribute(eVertexAttribute_Position0, "in_pos_frame0");
        mGpuProgram->BindAttribute(eVertexAttribute_Position1, "in_pos_frame1");
        mGpuProgram->BindAttribute(eVertexAttribute_Texcoord0, "in_texcoord");
    }

    void OnProgramFree() override
    {
        mUniformID_view_projection_matrix = GpuVariable_NULL;
        mUniformID_model_matrix = GpuVariable_NULL;
        mUniformID_mix_frames = GpuVariable_NULL;
    }

private:
    // render constants
    GpuVariableLocation mUniformID_view_projection_matrix = GpuVariable_NULL;
    GpuVariableLocation mUniformID_model_matrix = GpuVariable_NULL;
    GpuVariableLocation mUniformID_mix_frames = GpuVariable_NULL;
};

