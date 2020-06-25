#include "pch.h"
#include "RenderableWaterLavaMesh.h"
#include "WaterLavaMeshRenderer.h"
#include "RenderManager.h"
#include "SceneObject.h"
#include "TerrainTile.h"

RenderableWaterLavaMesh::RenderableWaterLavaMesh()
    : mWaveWidth()
    , mWaveHeight()
    , mWaveFreq()
    , mWaveTime()
{
    mDebugColor = Color32_Cyan;
}

void RenderableWaterLavaMesh::ReleaseRenderResources()
{
    WaterLavaMeshRenderer& renderer = gRenderManager.mWaterLavaMeshRenderer;
    renderer.ReleaseRenderdata(this);

    InvalidateMesh();
}

void RenderableWaterLavaMesh::SetWaterLavaTiles(const TilesList& tilesArray)
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
    SetLocalBoundingBox(bounds);
    InvalidateMesh();
}

void RenderableWaterLavaMesh::SetSurfaceParams(float translucency, float waveWidth, float waveHeight, float waveFreq, float waterlineHeight)
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

void RenderableWaterLavaMesh::SetSurfaceTexture(Texture2D* diffuseTexture)
{
    SetMeshMaterialsCount(1);

    MeshMaterial* meshMaterial = GetMeshMaterial();
    meshMaterial->mDiffuseTexture = diffuseTexture;
}

void RenderableWaterLavaMesh::UpdateFrame(float deltaTime)
{
    mWaveTime += mWaveFreq * deltaTime;
}

void RenderableWaterLavaMesh::RenderFrame(SceneRenderContext& renderContext)
{
    WaterLavaMeshRenderer& renderer = gRenderManager.mWaterLavaMeshRenderer;
    renderer.Render(renderContext, this);
}

void RenderableWaterLavaMesh::PrepareRenderResources()
{
    if (!IsMeshInvalidated())
        return;

    WaterLavaMeshRenderer& renderer = gRenderManager.mWaterLavaMeshRenderer;
    renderer.PrepareRenderdata(this);
}