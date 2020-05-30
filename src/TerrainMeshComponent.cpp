#include "pch.h"
#include "TerrainMeshComponent.h"
#include "GameObject.h"
#include "GraphicsDevice.h"
#include "MapTile.h"
#include "GameWorld.h"
#include "GpuBuffer.h"
#include "TransformComponent.h"
#include "TerrainMeshRenderer.h"
#include "RenderManager.h"

TerrainMeshComponent::TerrainMeshComponent(GameObject* gameObject) 
    : RenderableComponent(gameObject)
    , mMeshInvalidated()
{
    debug_assert(mGameObject);
    mGameObject->mDebugColor = Color32_Brown;
}

void TerrainMeshComponent::RenderFrame(SceneRenderContext& renderContext)
{
    if (IsMeshInvalidated())
    {
        PrepareRenderResources();
    }
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
    sectorBox.mMin.x = (mMapTerrainRect.x * DUNGEON_CELL_SIZE) - DUNGEON_CELL_HALF_SIZE;
    sectorBox.mMin.y = 0.0f;
    sectorBox.mMin.z = (mMapTerrainRect.y * DUNGEON_CELL_SIZE) - DUNGEON_CELL_HALF_SIZE;
    // max
    sectorBox.mMax.x = sectorBox.mMin.x + (mMapTerrainRect.w * DUNGEON_CELL_SIZE);
    sectorBox.mMax.y = 3.0f;
    sectorBox.mMax.z = sectorBox.mMin.z + (mMapTerrainRect.h * DUNGEON_CELL_SIZE);

    TransformComponent* transformComponent = mGameObject->mTransformComponent;
    transformComponent->SetLocalBoundingBox(sectorBox);

    InvalidateMesh();
}

void TerrainMeshComponent::InvalidateMesh()
{
    mMeshInvalidated = true;
}

bool TerrainMeshComponent::IsMeshInvalidated() const
{
    return mMeshInvalidated;
}

void TerrainMeshComponent::PrepareRenderResources()
{
}

void TerrainMeshComponent::ReleaseRenderResources()
{
}