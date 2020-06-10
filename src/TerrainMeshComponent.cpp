#include "pch.h"
#include "TerrainMeshComponent.h"
#include "Entity.h"
#include "GraphicsDevice.h"
#include "TerrainTile.h"
#include "GameWorld.h"
#include "GpuBuffer.h"
#include "TransformComponent.h"
#include "TerrainMeshRenderer.h"
#include "RenderManager.h"

TerrainMeshComponent::TerrainMeshComponent(Entity* entity) 
    : RenderableComponent(entity)
{
    debug_assert(mParentEntity);
    mParentEntity->mDebugColor = Color32_Brown;
}

void TerrainMeshComponent::RenderFrame(SceneRenderContext& renderContext)
{
    TerrainMeshRenderer& renderer = gRenderManager.mTerrainMeshRenderer;
    renderer.Render(renderContext, this);
}

void TerrainMeshComponent::SetTerrainArea(const Rectangle& mapArea)
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

    TransformComponent* transformComponent = mParentEntity->mTransformComponent;
    transformComponent->SetLocalBoundingBox(sectorBox);

    InvalidateMesh();
}

void TerrainMeshComponent::PrepareRenderResources()
{
    if (!IsMeshInvalidated())
        return;

    TerrainMeshRenderer& renderer = gRenderManager.mTerrainMeshRenderer;
    renderer.PrepareRenderdata(this);
}

void TerrainMeshComponent::ReleaseRenderResources()
{
    TerrainMeshRenderer& renderer = gRenderManager.mTerrainMeshRenderer;
    renderer.ReleaseRenderdata(this);

    InvalidateMesh();
}