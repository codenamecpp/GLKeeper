#include "pch.h"
#include "WaterLavaMesh.h"
#include "SceneRenderList.h"

WaterLavaMesh::WaterLavaMesh()
    : mVertexCount()
    , mTriangleCount()
    , mMeshDirty()
    , mWaveTime()
{
}

WaterLavaMesh::~WaterLavaMesh()
{
    DestroyRenderData();
}

void WaterLavaMesh::SetWaterLavaTiles(const TilesArray& tilesArray)
{
    mWaterLavaTiles = tilesArray;
    mMeshDirty = true;
}

void WaterLavaMesh::SetSurfaceParams(float translucency, float waveWidth, float waveHeight, float waveFreq, float waterlineHeight)
{
    mTranslucency = translucency;
    mWaveWidth = waveWidth;
    mWaveHeight = waveHeight;
    mWaveFreq = waveFreq;
    mWaterlineHeight = waterlineHeight;
}

void WaterLavaMesh::UpdateMesh()
{

}

void WaterLavaMesh::UpdateFrame(float deltaTime)
{
    // todo
}

void WaterLavaMesh::RegisterForRendering(SceneRenderList& renderList)
{
    if (mTranslucency < 1.0f)
    {
        renderList.RegisterObject(eRenderPass_Translucent, this);
    }
    else
    {
        renderList.RegisterObject(eRenderPass_Opaque, this);
    }
}

void WaterLavaMesh::PrepareRenderData()
{

}

void WaterLavaMesh::DestroyRenderData()
{

}
