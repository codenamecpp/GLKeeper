#include "pch.h"
#include "WaterLavaMeshComponent.h"
#include "TransformComponent.h"
#include "WaterLavaMeshRenderer.h"
#include "RenderManager.h"
#include "Entity.h"
#include "TerrainTile.h"

WaterLavaMeshComponent::WaterLavaMeshComponent(Entity* entity)
    : RenderableComponent(entity)
    , mWaveWidth()
    , mWaveHeight()
    , mWaveFreq()
    , mWaveTime()
{
    debug_assert(mParentEntity);
    mParentEntity->mDebugColor = Color32_Cyan;
}

void WaterLavaMeshComponent::ReleaseRenderResources()
{
    WaterLavaMeshRenderer& renderer = gRenderManager.mWaterLavaMeshRenderer;
    renderer.ReleaseRenderdata(this);

    InvalidateMesh();
}

void WaterLavaMeshComponent::SetWaterLavaTiles(const TilesList& tilesArray)
{
    mWaterLavaTiles = tilesArray;

    // setup bounds
    cxx::aabbox bounds;
    for (TerrainTile* currentTile: tilesArray)
    {
        cxx::aabbox currentTileBounds;
        GetTerrainBlockBounds(currentTile->mTileLocation, currentTileBounds);

        bounds.extend(currentTileBounds);
    }
    TransformComponent* transformComponent = mParentEntity->mTransform;
    transformComponent->SetLocalBoundingBox(bounds);

    InvalidateMesh();
}

void WaterLavaMeshComponent::SetSurfaceParams(float translucency, float waveWidth, float waveHeight, float waveFreq, float waterlineHeight)
{
    // setup params
    mTranslucency = translucency;
    mWaveWidth = waveWidth;
    mWaveHeight = waveHeight;
    mWaveFreq = waveFreq;
    mWaterlineHeight = waterlineHeight;

    // setup material
    SetMeshMaterialsCount(1);

    MeshMaterial* meshMaterial = GetMeshMaterial();
    if (translucency < 1.0f)
    {
        meshMaterial->mRenderStates.mIsAlphaBlendEnabled = true;
        meshMaterial->mRenderStates.mBlendingMode = eBlendingMode_Alpha;
    }
    else
    {
        meshMaterial->mRenderStates.mIsAlphaBlendEnabled = false;
    }
    meshMaterial->mMaterialColor.mA = (unsigned char)(mTranslucency * 255);
}

void WaterLavaMeshComponent::SetSurfaceTexture(Texture2D* diffuseTexture)
{
    SetMeshMaterialsCount(1);

    MeshMaterial* meshMaterial = GetMeshMaterial();
    meshMaterial->mDiffuseTexture = diffuseTexture;
}

void WaterLavaMeshComponent::UpdateComponent(float deltaTime)
{
    mWaveTime += mWaveFreq * deltaTime;
}

void WaterLavaMeshComponent::RenderFrame(SceneRenderContext& renderContext)
{
    WaterLavaMeshRenderer& renderer = gRenderManager.mWaterLavaMeshRenderer;
    renderer.Render(renderContext, this);
}

void WaterLavaMeshComponent::PrepareRenderResources()
{
    if (!IsMeshInvalidated())
        return;

    WaterLavaMeshRenderer& renderer = gRenderManager.mWaterLavaMeshRenderer;
    renderer.PrepareRenderdata(this);
}