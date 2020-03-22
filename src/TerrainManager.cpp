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
    BuildFullTerrainMesh();

    return true;
}

void TerrainManager::Deinit()
{
    DestroyTerrainMeshList();

    mDirtyTilesArray.clear();
}

void TerrainManager::CreateTerrainMeshList()
{
    debug_assert(mTerrainMeshArray.empty());

    int mapsizex = gGameWorld.mMapData.mDimensions.x;
    int mapsizey = gGameWorld.mMapData.mDimensions.y;

    int numMeshesPerWidth = mapsizex / TerrainMeshSizeTiles;
    if ((mapsizex % TerrainMeshSizeTiles) > 0)
    {
        ++numMeshesPerWidth;
    }

    int numMeshesPerHeight = mapsizey / TerrainMeshSizeTiles;
    if ((mapsizey % TerrainMeshSizeTiles) > 0)
    {
        ++numMeshesPerHeight;
    }

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

void TerrainManager::UpdateTerrainMesh()
{
    if (mDirtyTilesArray.empty())
        return;

    std::set<GenericRoom*> invalidateRooms;

    // build terrain tiles and collect invalidated rooms
    for (MapTile* currentTile: mDirtyTilesArray)
    {
        if (currentTile->mRoomInstance)
        {
            invalidateRooms.insert(currentTile->mRoomInstance);
        }

        // traverse each invalidated tile face
        for (TileFaceData& currTileFace: currentTile->mFaces)
        {
            // rooms should rebuild their walls
            //if (GenericRoom* invalidateRoom = currTileFace.mWallSectionData)
            //{
            //    invalidateRooms.insert(invalidateRoom);
            //}
            // todo

        }
        // force rebuild mesh
        gGameWorld.mDungeonBuilder.BuildTerrainMesh(currentTile);
    }

    // ask rooms rebuild themselves
    for (GenericRoom* currentRoom: invalidateRooms)
    {
        //currentRoom->RefreshGeometries(false); // todo
    }

    // todo
    // refresh heightfield last
    //for (GameMapTile* currentTile: mInvalidateTilesList)
    //{
    //    gWorldState.mTerrainHeightField.UpdateHeights(currentTile);
    //    // reset invalidation flag
    //    for (TileFaceData& tileface: currentTile->mFaces)
    //    {
    //        tileface.mInvalidated = false;
    //    }
    //}

    // update terrain meshes
    for (TerrainMesh* currTerrainMesh: mTerrainMeshArray)
    {
        currTerrainMesh->UpdateMesh();
    }

    mDirtyTilesArray.clear();
}

void TerrainManager::BuildFullTerrainMesh()
{
    mDirtyTilesArray.clear();

    int dimsx = gGameWorld.mMapData.mDimensions.x;
    int dimsy = gGameWorld.mMapData.mDimensions.y;

    // build terrain tiles and collect invalidated rooms
    for (int tiley = 0; tiley < dimsy; ++tiley)
    for (int tilex = 0; tilex < dimsx; ++tilex)
    {
        Point2D currTilePosition (tilex, tiley);

        MapTile* currentTile = gGameWorld.mMapData.GetMapTile(currTilePosition);
        debug_assert(currentTile);

        gGameWorld.mDungeonBuilder.BuildTerrainMesh(currentTile);
    }

    // update terrain meshes
    for (TerrainMesh* currTerrainMesh: mTerrainMeshArray)
    {
        currTerrainMesh->UpdateMesh();
    }
}