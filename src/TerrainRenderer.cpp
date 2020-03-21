#include "pch.h"
#include "TerrainRenderer.h"

bool TerrainRenderer::Initialize()
{
    if (!mTerrainRenderProgram.LoadProgram())
    {
        Deinit();
        return false;
    }
    return true;
}

void TerrainRenderer::Deinit()
{
    mTerrainRenderProgram.FreeProgram();
}

void TerrainRenderer::RenderTerrainMesh(SceneRenderContext& renderContext, TerrainMesh* terrainMesh)
{
}