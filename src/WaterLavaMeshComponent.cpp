#include "pch.h"
#include "WaterLavaMeshComponent.h"
#include "TransformComponent.h"
#include "WaterLavaMeshRenderer.h"
#include "RenderManager.h"
#include "GameObject.h"
#include "TerrainTile.h"

WaterLavaMeshComponent::WaterLavaMeshComponent(GameObject* gameObject)
    : RenderableComponent(gameObject)
    , mWaveWidth()
    , mWaveHeight()
    , mWaveFreq()
    , mWaveTime()
    , mMeshInvalidated()
{
    debug_assert(mGameObject);
    mGameObject->mDebugColor = Color32_Cyan;
}

void WaterLavaMeshComponent::InvalidateMesh()
{
    mMeshInvalidated = true;
}

bool WaterLavaMeshComponent::IsMeshInvalidated() const
{
    return mMeshInvalidated;
}

void WaterLavaMeshComponent::ReleaseRenderResources()
{
    WaterLavaMeshRenderer& renderer = gRenderManager.mWaterLavaMeshRenderer;
    renderer.ReleaseRenderdata(this);

    InvalidateMesh();
}

void WaterLavaMeshComponent::SetWaterLavaTiles(const TilesArray& tilesArray)
{
    mWaterLavaTiles = tilesArray;

    // setup bounds
    cxx::aabbox bounds;
    for (TerrainTile* currentTile: tilesArray)
    {
        cxx::aabbox currentTileBounds;
        GetMapBlockBounds(currentTile->mTileLocation, currentTileBounds);

        bounds.extend(currentTileBounds);
    }
    TransformComponent* transformComponent = mGameObject->mTransformComponent;
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
    if (IsMeshInvalidated())
    {
        PrepareRenderResources();
    }

    WaterLavaMeshRenderer& renderer = gRenderManager.mWaterLavaMeshRenderer;
    renderer.Render(renderContext, this);
}

void WaterLavaMeshComponent::PrepareRenderResources()
{
    if (!IsMeshInvalidated())
        return;

    mMeshInvalidated = false;

    WaterLavaMeshRenderer& renderer = gRenderManager.mWaterLavaMeshRenderer;
    renderer.PrepareRenderdata(this);
}