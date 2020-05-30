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
    
    void SetMixFrames(float mixFrames)
    {
        debug_assert(IsProgramLoaded());
        mGpuProgram->SetUniformParam(mUniformID_mix_frames, mixFrames);
    }

private:
    void HandleProgramLoad() override
    {
        mUniformID_mix_frames = mGpuProgram->QueryUniformLocation("mix_frames");
        debug_assert(mUniformID_mix_frames != GpuVariable_NULL);

        // configure input layout
        mGpuProgram->BindAttribute(eVertexAttribute_Position0, "in_pos_frame0");
        mGpuProgram->BindAttribute(eVertexAttribute_Position1, "in_pos_frame1");
        mGpuProgram->BindAttribute(eVertexAttribute_Texcoord0, "in_texcoord");
    }

    void HandleProgramFree() override
    {
        mUniformID_mix_frames = GpuVariable_NULL;
    }

private:
    // render constants
    GpuVariableLocation mUniformID_mix_frames = GpuVariable_NULL;
};

