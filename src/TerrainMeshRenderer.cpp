#include "pch.h"
#include "TerrainMeshRenderer.h"

bool TerrainMeshRenderer::Initialize()
{
    if (!mTerrainRenderProgram.LoadProgram())
    {
        Deinit();
        return false;
    }
    return true;
}

void TerrainMeshRenderer::Deinit()
{
    mTerrainRenderProgram.FreeProgram();
}

void TerrainMeshRenderer::RenderTerrainMesh(SceneRenderContext& renderContext, TerrainMesh* terrainMesh)
{
}