#include "pch.h"
#include "TerrainManager.h"
#include "GameWorld.h"
#include "Console.h"
#include "RenderScene.h"
#include "TerrainMesh.h"

const int TerrainMeshSizeTiles = 8; // 8x8 tiles per terrain mesh

bool TerrainManager::Initialize()
{
    int mapsizex = gGameWorld.mMapData.mDimensions.x;
    int mapsizey = gGameWorld.mMapData.mDimensions.y;

    if (mapsizex == 0 || mapsizey == 0)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Game map has invalid dimensions: %dx%d", mapsizex, mapsizey);
        return false;
    }

    CreateTerrainMeshList();

    return true;
}

void TerrainManager::Deinit()
{
    DestroyTerrainMeshList();
}

void TerrainManager::CreateTerrainMeshList()
{
    debug_assert(mTerrainMeshArray.empty());

    int mapsizex = gGameWorld.mMapData.mDimensions.x;
    int mapsizey = gGameWorld.mMapData.mDimensions.y;

    int numMeshesPerWidth = mapsizex / TerrainMeshSizeTiles;
    int numMeshesPerHeight = mapsizey / TerrainMeshSizeTiles;

    for (int imeshy = 0; imeshy < numMeshesPerHeight; ++imeshy)
    for (int imeshx = 0; imeshx < numMeshesPerWidth; ++imeshx)
    {
        Rect2D rcmap;
        rcmap.mX = imeshx * TerrainMeshSizeTiles;
        rcmap.mY = imeshy * TerrainMeshSizeTiles;
        rcmap.mSizeX = TerrainMeshSizeTiles;
        rcmap.mSizeY = TerrainMeshSizeTiles;
        if (rcmap.mX + TerrainMeshSizeTiles > mapsizex)
        {
            rcmap.mSizeX = mapsizex - rcmap.mX;
        }
        if (rcmap.mY + TerrainMeshSizeTiles > mapsizey)
        {
            rcmap.mSizeY = mapsizey - rcmap.mY;
        }

        TerrainMesh* currTerrainMesh = gRenderScene.CreateTerrainMesh(rcmap);
        gRenderScene.AttachObject(currTerrainMesh);

        mTerrainMeshArray.push_back(currTerrainMesh);
    }
}

void TerrainManager::DestroyTerrainMeshList()
{
    for (TerrainMesh* currTerrainMesh: mTerrainMeshArray)
    {
        gRenderScene.DetachObject(currTerrainMesh);
        gRenderScene.DestroyObject(currTerrainMesh);
    }
    mTerrainMeshArray.clear();
}