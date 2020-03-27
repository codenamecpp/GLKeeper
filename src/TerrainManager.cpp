#include "pch.h"
#include "TerrainManager.h"
#include "GameWorld.h"
#include "Console.h"
#include "RenderScene.h"
#include "TerrainMesh.h"
#include "WaterLavaMesh.h"
#include "RoomsManager.h"
#include "GenericRoom.h"

const int TerrainMeshSizeTiles = 8; // 8x8 tiles per terrain mesh

//////////////////////////////////////////////////////////////////////////

TerrainManager gTerrainManager;

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
    CreateWaterLavaMeshList();

    return true;
}

void TerrainManager::Deinit()
{
    DestroyWaterLavaMeshList();
    DestroyTerrainMeshList();

    mInvalidatedTiles.clear();
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
    if (mInvalidatedTiles.empty())
        return;

    std::set<GenericRoom*> invalidateRooms;

    // build terrain tiles and collect invalidated rooms
    for (MapTile* currentTile: mInvalidatedTiles)
    {
        if (currentTile->mRoom)
        {
            invalidateRooms.insert(currentTile->mRoom);
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

    // reset invalidated flag
    for (MapTile* currentTile: mInvalidatedTiles)
    {
        currentTile->mIsMeshInvalidated = false;
    }

    // update terrain mesh objects
    for (TerrainMesh* currTerrainMesh: mTerrainMeshArray)
    {
        currTerrainMesh->UpdateMesh();
    }

    mInvalidatedTiles.clear();
}

void TerrainManager::HandleTileMeshInvalidated(MapTile* mapTile)
{
    if (mapTile == nullptr)
    {
        debug_assert(false);
        return;
    }

    cxx::push_back_if_unique(mInvalidatedTiles, mapTile);
}

void TerrainManager::BuildFullTerrainMesh()
{
    mInvalidatedTiles.clear();

    int dimsx = gGameWorld.mMapData.mDimensions.x;
    int dimsy = gGameWorld.mMapData.mDimensions.y;

    // build terrain tiles
    for (int tiley = 0; tiley < dimsy; ++tiley)
    for (int tilex = 0; tilex < dimsx; ++tilex)
    {
        Point2D currTilePosition (tilex, tiley);

        MapTile* currentTile = gGameWorld.mMapData.GetMapTile(currTilePosition);
        debug_assert(currentTile);

        gGameWorld.mDungeonBuilder.BuildTerrainMesh(currentTile);
        currentTile->mIsMeshInvalidated = false;
    }

    // ask rooms to build its tiles
    for (GenericRoom* currentRoom: gRoomsManager.mRoomsList)
    {
        currentRoom->BuildTilesMesh();
    }

    // update terrain mesh objects
    for (TerrainMesh* currTerrainMesh: mTerrainMeshArray)
    {
        currTerrainMesh->UpdateMesh();
    }
}

void TerrainManager::CreateWaterLavaMeshList()
{
    TilesArray lavaTiles;
    TilesArray waterTiles;

    lavaTiles.reserve(128);
    waterTiles.reserve(128);

    // find water and lava tiles
    for (int tiley = 0; tiley < gGameWorld.mMapData.mDimensions.y; ++tiley)
    for (int tilex = 0; tilex < gGameWorld.mMapData.mDimensions.x; ++tilex)
    {
        Point2D currTilePosition (tilex, tiley);

        MapTile* currMapTile = gGameWorld.mMapData.GetMapTile(currTilePosition);

        // collect lava tile
        if (currMapTile->mBaseTerrain->mIsLava)
        {
            lavaTiles.push_back(currMapTile);
        }

        // collect water tile
        if (currMapTile->mBaseTerrain->mIsWater)
        {
            waterTiles.push_back(currMapTile);
        }
    }

    MapFloodFillFlags floodfillFlags;
    floodfillFlags.mSameBaseTerrain = true;
    floodfillFlags.mSameOwner = false;

    TilesArray tempTilesArray;

    // create lava surfaces
    while (!lavaTiles.empty())
    {
        MapTile* originTile = lavaTiles.back();
        lavaTiles.pop_back();

        tempTilesArray.clear();
        gGameWorld.mMapData.FloodFill4(tempTilesArray, originTile, floodfillFlags);
        if (tempTilesArray.empty())
        {
            debug_assert(false);
            continue;
        }

        WaterLavaMesh* lavaMeshObject = gRenderScene.CreateLavaMesh(tempTilesArray);
        gRenderScene.AttachObject(lavaMeshObject);
        mWaterLavaMeshArray.push_back(lavaMeshObject);

        // remove used tiles
        cxx::erase_elements(lavaTiles, tempTilesArray);
    }

    // create water surfaces
    while (!waterTiles.empty())
    {
        MapTile* originTile = waterTiles.back();
        waterTiles.pop_back();

        tempTilesArray.clear();
        gGameWorld.mMapData.FloodFill4(tempTilesArray, originTile, floodfillFlags);
        if (tempTilesArray.empty())
        {
            debug_assert(false);
            continue;
        }

        WaterLavaMesh* waterMeshObject = gRenderScene.CreateWaterMesh(tempTilesArray);
        gRenderScene.AttachObject(waterMeshObject);
        mWaterLavaMeshArray.push_back(waterMeshObject);

        // remove used tiles
        cxx::erase_elements(waterTiles, tempTilesArray);
    }

    // force build mesh
    for (WaterLavaMesh* currMesh: mWaterLavaMeshArray)
    {
        currMesh->UpdateMesh();
    }
}

void TerrainManager::DestroyWaterLavaMeshList()
{
    for (WaterLavaMesh* currWaterLavaMesh: mWaterLavaMeshArray)
    {
        gRenderScene.DetachObject(currWaterLavaMesh);
        gRenderScene.DestroyObject(currWaterLavaMesh);
    }
    mWaterLavaMeshArray.clear();
}