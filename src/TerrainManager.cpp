#include "pch.h"
#include "TerrainManager.h"
#include "GameWorld.h"
#include "Console.h"
#include "RenderScene.h"
#include "RoomsManager.h"
#include "GenericRoom.h"
#include "GameObject.h"
#include "GameObjectsManager.h"
#include "TexturesManager.h"
#include "TerrainMeshComponent.h"
#include "WaterLavaMeshComponent.h"

const int TerrainMeshSizeTiles = 8; // 8x8 tiles per terrain mesh

//////////////////////////////////////////////////////////////////////////

TerrainManager gTerrainManager;

bool TerrainManager::Initialize()
{

    return true;
}

void TerrainManager::Deinit()
{

}

void TerrainManager::EnterWorld()
{
    int mapsizex = gGameWorld.mMapData.mDimensions.x;
    int mapsizey = gGameWorld.mMapData.mDimensions.y;

    if (mapsizex == 0 || mapsizey == 0)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Game map has invalid dimensions: %dx%d", mapsizex, mapsizey);
        return;
    }

    CreateTerrainMeshList();    
    CreateWaterLavaMeshList();
}

void TerrainManager::ClearWorld()
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
        Rectangle rcmap;
        rcmap.x = imeshx * TerrainMeshSizeTiles;
        rcmap.y = imeshy * TerrainMeshSizeTiles;
        rcmap.w = TerrainMeshSizeTiles;
        rcmap.h = TerrainMeshSizeTiles;
        if (rcmap.x + TerrainMeshSizeTiles > mapsizex)
        {
            rcmap.w = mapsizex - rcmap.x;
        }
        if (rcmap.y + TerrainMeshSizeTiles > mapsizey)
        {
            rcmap.h = mapsizey - rcmap.y;
        }

        GameObject* gameObject = CreateObjectTerrain(rcmap);
        gRenderScene.AttachObject(gameObject);
    }
}

void TerrainManager::DestroyTerrainMeshList()
{
    for (GameObject* currTerrainMesh: mTerrainMeshArray)
    {
        gRenderScene.DetachObject(currTerrainMesh);
        gGameObjectsManager.DestroyGameObject(currTerrainMesh);
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
        if (currentTile->mBuiltRoom)
        {
            invalidateRooms.insert(currentTile->mBuiltRoom);
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
    for (GameObject* currTerrainMesh: mTerrainMeshArray)
    {
        TerrainMeshComponent* meshComponent = currTerrainMesh->GetComponent<TerrainMeshComponent>();
        debug_assert(meshComponent);

        meshComponent->PrepareRenderResources();
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

void TerrainManager::ClearInvalidated()
{
    for (MapTile* currTile: mInvalidatedTiles)
    {
        currTile->mIsMeshInvalidated = false;
    }
    mInvalidatedTiles.clear();
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
        Point currTilePosition (tilex, tiley);

        MapTile* currentTile = gGameWorld.mMapData.GetMapTile(currTilePosition);
        debug_assert(currentTile);

        gGameWorld.mDungeonBuilder.BuildTerrainMesh(currentTile);
    }

    // ask rooms to build its tiles
    for (GenericRoom* currentRoom: gRoomsManager.mRoomsList)
    {
        currentRoom->BuildTilesMesh();
    }

    ClearInvalidated();

    // update terrain mesh objects
    for (GameObject* currTerrainMesh: mTerrainMeshArray)
    {
        TerrainMeshComponent* meshComponent = currTerrainMesh->GetComponent<TerrainMeshComponent>();
        debug_assert(meshComponent);

        meshComponent->PrepareRenderResources();
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
        Point currTilePosition (tilex, tiley);

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

        GameObject* lavaMeshObject = CreateObjectLava(tempTilesArray);
        gRenderScene.AttachObject(lavaMeshObject);

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

        GameObject* waterMeshObject = CreateObjectWater(tempTilesArray);
        gRenderScene.AttachObject(waterMeshObject);

        // remove used tiles
        cxx::erase_elements(waterTiles, tempTilesArray);
    }

    // force build mesh
    for (GameObject* currMesh: mWaterLavaMeshArray)
    {
        WaterLavaMeshComponent* meshComponent = currMesh->GetComponent<WaterLavaMeshComponent>();
        debug_assert(meshComponent);

        meshComponent->PrepareRenderResources();
    }
}

void TerrainManager::DestroyWaterLavaMeshList()
{
    for (GameObject* currWaterLavaMesh: mWaterLavaMeshArray)
    {
        gRenderScene.DetachObject(currWaterLavaMesh);
        gGameObjectsManager.DestroyGameObject(currWaterLavaMesh);
    }
    mWaterLavaMeshArray.clear();
}

GameObject* TerrainManager::CreateObjectTerrain(const Rectangle& mapArea)
{
    GameObject* gameObject = gGameObjectsManager.CreateGameObject();
    debug_assert(gameObject);
    mTerrainMeshArray.push_back(gameObject);

    TerrainMeshComponent* meshComponent = new TerrainMeshComponent(gameObject);
    gameObject->AddComponent(meshComponent);

    meshComponent->SetTerrainArea(mapArea);

    return gameObject;
}

GameObject* TerrainManager::CreateObjectLava(const TilesArray& tilesArray)
{
    GameObject* gameObject = gGameObjectsManager.CreateGameObject();
    debug_assert(gameObject);
    mWaterLavaMeshArray.push_back(gameObject);

    WaterLavaMeshComponent* meshComponent = new WaterLavaMeshComponent(gameObject);
    gameObject->AddComponent(meshComponent);

    meshComponent->SetWaterLavaTiles(tilesArray);
    meshComponent->SetSurfaceTexture(gTexturesManager.mLavaTexture);
    meshComponent->SetSurfaceParams(DEFAULT_LAVA_TRANSLUCENCY, DEFAULT_LAVA_WAVE_WIDTH, DEFAULT_LAVA_WAVE_HEIGHT, DEFAULT_LAVA_WAVE_FREQ, DEFAULT_LAVA_LEVEL);

    return gameObject;
}

GameObject* TerrainManager::CreateObjectWater(const TilesArray& tilesArray)
{
    GameObject* gameObject = gGameObjectsManager.CreateGameObject();
    debug_assert(gameObject);
    mWaterLavaMeshArray.push_back(gameObject);

    WaterLavaMeshComponent* meshComponent = new WaterLavaMeshComponent(gameObject);
    gameObject->AddComponent(meshComponent);

    meshComponent->SetWaterLavaTiles(tilesArray);
    meshComponent->SetSurfaceTexture(gTexturesManager.mWaterTexture);
    meshComponent->SetSurfaceParams(DEFAULT_WATER_TRANSLUCENCY, DEFAULT_WATER_WAVE_WIDTH, DEFAULT_WATER_WAVE_HEIGHT, DEFAULT_WATER_WAVE_FREQ, DEFAULT_WATER_LEVEL);

    return gameObject;
}