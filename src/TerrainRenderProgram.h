#pragma once

#include "RenderProgram.h"
#include "GpuProgram.h"

// terrain mesh shader
class TerrainRenderProgram: public RenderProgram
{
public:
    TerrainRenderProgram(): RenderProgram("shaders/terrain.glsl")
    {
    }

private:
    void HandleProgramLoad() override
    {
        // configure input layout
        mGpuProgram->BindAttribute(eVertexAttribute_Position0, "in_pos");
        //mGpuProgram->BindAttribute(eVertexAttribute_Normal0, "in_normal");
        mGpuProgram->BindAttribute(eVertexAttribute_Texcoord0, "in_texcoord");
        mGpuProgram->BindAttribute(eVertexAttribute_TerrainTilePosition, "in_tile_coord");
    }

    void HandleProgramFree() override
    {
    }

private:
    // render constants
};