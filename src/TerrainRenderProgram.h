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

private:
    void OnProgramLoad() override
    {
        mUniformID_view_projection_matrix = mGpuProgram->QueryUniformLocation("view_projection_matrix");
        debug_assert(mUniformID_view_projection_matrix != GpuVariable_NULL);

        mUniformID_model_matrix = mGpuProgram->QueryUniformLocation("model_matrix");
        debug_assert(mUniformID_model_matrix != GpuVariable_NULL);

        // configure input layout
        mGpuProgram->BindAttribute(eVertexAttribute_Position0, "in_pos");
        mGpuProgram->BindAttribute(eVertexAttribute_Normal0, "in_normal");
        mGpuProgram->BindAttribute(eVertexAttribute_Texcoord0, "in_texcoord");
        mGpuProgram->BindAttribute(eVertexAttribute_TerrainTilePosition, "in_tile_coord");
    }

    void OnProgramFree() override
    {
        mUniformID_view_projection_matrix = GpuVariable_NULL;
        mUniformID_model_matrix = GpuVariable_NULL;
    }

private:
    // render constants
    GpuVariableLocation mUniformID_view_projection_matrix = GpuVariable_NULL;
    GpuVariableLocation mUniformID_model_matrix = GpuVariable_NULL;
};