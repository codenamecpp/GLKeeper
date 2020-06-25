#include "pch.h"
#include "RenderableTerrainMesh.h"
#include "SceneObject.h"
#include "GraphicsDevice.h"
#include "TerrainTile.h"
#include "GameWorld.h"
#include "GpuBuffer.h"
#include "TerrainMeshRenderer.h"
#include "RenderManager.h"

RenderableTerrainMesh::RenderableTerrainMesh() 
{
    mDebugColor = Color32_Brown;
}

void RenderableTerrainMesh::RenderFrame(SceneRenderContext& renderContext)
{
    TerrainMeshRenderer& renderer = gRenderManager.mTerrainMeshRenderer;
    renderer.Render(renderContext, this);
}

void RenderableTerrainMesh::SetTerrainArea(const Rectangle& mapArea)
{
    if (mMapTerrainRect == mapArea)
        return;

    mMapTerrainRect = mapArea;

    cxx::aabbox sectorBox;
    // min
    sectorBox.mMin.x = (mMapTerrainRect.x * TERRAIN_BLOCK_SIZE) - TERRAIN_BLOCK_HALF_SIZE;
    sectorBox.mMin.y = 0.0f;
    sectorBox.mMin.z = (mMapTerrainRect.y * TERRAIN_BLOCK_SIZE) - TERRAIN_BLOCK_HALF_SIZE;
    // max
    sectorBox.mMax.x = sectorBox.mMin.x + (mMapTerrainRect.w * TERRAIN_BLOCK_SIZE);
    sectorBox.mMax.y = 3.0f;
    sectorBox.mMax.z = sectorBox.mMin.z + (mMapTerrainRect.h * TERRAIN_BLOCK_SIZE);

    SetLocalBoundingBox(sectorBox);
    InvalidateMesh();
}

void RenderableTerrainMesh::PrepareRenderResources()
{
    if (!IsMeshInvalidated())
        return;

    TerrainMeshRenderer& renderer = gRenderManager.mTerrainMeshRenderer;
    renderer.PrepareRenderdata(this);
}

void RenderableTerrainMesh::ReleaseRenderResources()
{
    TerrainMeshRenderer& renderer = gRenderManager.mTerrainMeshRenderer;
    renderer.ReleaseRenderdata(this);

    InvalidateMesh();
}