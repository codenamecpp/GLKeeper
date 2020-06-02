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
#include "Texture2D.h"

const int TerrainMeshSizeTiles = 8; // 8x8 tiles per terrain mesh
                                    
const Color32 TILE_TAGGED_COLOR = Color32::MakeRGBA(64, 64, 255, 0); // color constants

//////////////////////////////////////////////////////////////////////////

TerrainManager gTerrainManager;

bool TerrainManager::Initialize()
{
    return true;
}

void TerrainManager::Deinit()
{
    FreeHighhlightTilesTexture();
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

    InitTerrainMeshList();    
    InitWaterLavaMeshList();
    InitHighhlightTilesTexture();
}

void TerrainManager::ClearWorld()
{
    FreeHighhlightTilesTexture();
    FreeWaterLavaMeshList();
    FreeTerrainMeshList();

    mMeshInvalidatedTiles.clear();
    mHighlightTiles.clear();
}

void TerrainManager::PreRenderScene()
{
    if (!mHighlightTiles.empty())
    {
        UpdateHighhlightTilesTexture();
        mHighlightTiles.clear();
    }
}

void TerrainManager::InitTerrainMeshList()
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
        debug_assert(gameObject);
    }
}

void TerrainManager::FreeTerrainMeshList()
{
    for (GameObject* currTerrainMesh: mTerrainMeshArray)
    {
        gGameObjectsManager.DestroyGameObject(currTerrainMesh);
    }
    mTerrainMeshArray.clear();
}

void TerrainManager::UpdateTerrainMesh()
{
    if (mMeshInvalidatedTiles.empty())
        return;

    std::set<GenericRoom*> invalidateRooms;

    // build terrain tiles and collect invalidated rooms
    for (TerrainTile* currentTile: mMeshInvalidatedTiles)
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
    for (TerrainTile* currentTile: mMeshInvalidatedTiles)
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

    mMeshInvalidatedTiles.clear();
}

void TerrainManager::InvalidateTileMesh(TerrainTile* mapTile)
{
    if (mapTile && !mapTile->mIsMeshInvalidated)
    {
        if (cxx::contains(mMeshInvalidatedTiles, mapTile))
        {
            debug_assert(false);
        }
        mMeshInvalidatedTiles.push_back(mapTile);
        mapTile->mIsMeshInvalidated = true;
    }
    debug_assert(mapTile);
}

void TerrainManager::InvalidateTileNeighboursMesh(TerrainTile* mapTile)
{
    debug_assert(mapTile);
    if (mapTile)
    {
        for (TerrainTile* currTile: mapTile->mNeighbours)
        {
            InvalidateTileMesh(currTile);
        }
    }
    debug_assert(mapTile);
}

void TerrainManager::ClearInvalidatedTiles()
{
    for (TerrainTile* currTile: mMeshInvalidatedTiles)
    {
        currTile->mIsMeshInvalidated = false;
    }
    mMeshInvalidatedTiles.clear();
}

void TerrainManager::BuildFullTerrainMesh()
{
    mMeshInvalidatedTiles.clear();

    int dimsx = gGameWorld.mMapData.mDimensions.x;
    int dimsy = gGameWorld.mMapData.mDimensions.y;

    // build terrain tiles
    for (int tiley = 0; tiley < dimsy; ++tiley)
    for (int tilex = 0; tilex < dimsx; ++tilex)
    {
        Point currTilePosition (tilex, tiley);

        TerrainTile* currentTile = gGameWorld.mMapData.GetMapTile(currTilePosition);
        debug_assert(currentTile);

        gGameWorld.mDungeonBuilder.BuildTerrainMesh(currentTile);
    }

    // ask rooms to build its tiles
    for (GenericRoom* currentRoom: gRoomsManager.mRoomsList)
    {
        currentRoom->BuildTilesMesh();
    }

    ClearInvalidatedTiles();

    // update terrain mesh objects
    for (GameObject* currTerrainMesh: mTerrainMeshArray)
    {
        TerrainMeshComponent* meshComponent = currTerrainMesh->GetComponent<TerrainMeshComponent>();
        debug_assert(meshComponent);

        meshComponent->PrepareRenderResources();
    }
}

void TerrainManager::InitWaterLavaMeshList()
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

        TerrainTile* currMapTile = gGameWorld.mMapData.GetMapTile(currTilePosition);

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
        TerrainTile* originTile = lavaTiles.back();
        lavaTiles.pop_back();

        tempTilesArray.clear();
        gGameWorld.mMapData.FloodFill4(tempTilesArray, originTile, floodfillFlags);
        if (tempTilesArray.empty())
        {
            debug_assert(false);
            continue;
        }

        GameObject* lavaMeshObject = CreateObjectLava(tempTilesArray);
        debug_assert(lavaMeshObject);

        // remove used tiles
        cxx::erase_elements(lavaTiles, tempTilesArray);
    }

    // create water surfaces
    while (!waterTiles.empty())
    {
        TerrainTile* originTile = waterTiles.back();
        waterTiles.pop_back();

        tempTilesArray.clear();
        gGameWorld.mMapData.FloodFill4(tempTilesArray, originTile, floodfillFlags);
        if (tempTilesArray.empty())
        {
            debug_assert(false);
            continue;
        }

        GameObject* waterMeshObject = CreateObjectWater(tempTilesArray);
        debug_assert(waterMeshObject);

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

void TerrainManager::FreeWaterLavaMeshList()
{
    for (GameObject* currWaterLavaMesh: mWaterLavaMeshArray)
    {
        gGameObjectsManager.DestroyGameObject(currWaterLavaMesh);
    }
    mWaterLavaMeshArray.clear();
}

void TerrainManager::InitHighhlightTilesTexture()
{
    // allocate highlight tiles texture
    Point highlightImageSize = gGameWorld.mMapData.mDimensions;
    if (highlightImageSize.x > 0 && highlightImageSize.y > 0)
    {
        highlightImageSize.x = cxx::get_next_pot(highlightImageSize.x);
        highlightImageSize.y = cxx::get_next_pot(highlightImageSize.y);

        if (!mHighlightTilesImage.CreateImage(eTextureFormat_RGB8, highlightImageSize, 0, false))
        {
            debug_assert(false);

            gConsole.LogMessage(eLogMessage_Warning, "Cannot allocate tiles highhlight texture");
            return;
        }
        mHighlightTilesImage.FillWithColor(Color32_Black);

        // allocate hardware texture
        mHighlightTilesTexture = new Texture2D("highlight_terrain");
        if (mHighlightTilesTexture->CreateTexture(mHighlightTilesImage))
            return; // success

        // failed
        FreeHighhlightTilesTexture();

        debug_assert(false);
        gConsole.LogMessage(eLogMessage_Warning, "Cannot allocate tiles highhlight texture");
        return;
    }
    else
    {
        debug_assert(false);
    }
}

void TerrainManager::FreeHighhlightTilesTexture()
{
    SafeDelete(mHighlightTilesTexture);

    mHighlightTilesImage.Clear();
}

void TerrainManager::UpdateHighhlightTilesTexture()
{
    if (mHighlightTilesTexture == nullptr)
        return;

    unsigned char* pixels = mHighlightTilesImage.GetImageDataBuffer();

    for (TerrainTile* currentTile: mHighlightTiles)
    {
        int offset = currentTile->mTileLocation.y * mHighlightTilesImage.mTextureDesc.mDimensions.x + currentTile->mTileLocation.x;
        if (currentTile->mIsTagged)
        {
            pixels[offset * 3 + 0] = TILE_TAGGED_COLOR.mR;
            pixels[offset * 3 + 1] = TILE_TAGGED_COLOR.mG;
            pixels[offset * 3 + 2] = TILE_TAGGED_COLOR.mB;
        }
        else
        {
            pixels[offset * 3 + 0] = 0;
            pixels[offset * 3 + 1] = 0;
            pixels[offset * 3 + 2] = 0;
        }
    }
    mHighlightTilesTexture->UpdateTexture(0, pixels);
}

GameObject* TerrainManager::CreateObjectTerrain(const Rectangle& mapArea)
{
    GameObject* gameObject = gGameObjectsManager.CreateGameObject();
    debug_assert(gameObject);
    mTerrainMeshArray.push_back(gameObject);

    TerrainMeshComponent* meshComponent = gameObject->AddComponent<TerrainMeshComponent>();
    meshComponent->SetTerrainArea(mapArea);

    return gameObject;
}

GameObject* TerrainManager::CreateObjectLava(const TilesArray& tilesArray)
{
    GameObject* gameObject = gGameObjectsManager.CreateGameObject();
    debug_assert(gameObject);
    mWaterLavaMeshArray.push_back(gameObject);

    WaterLavaMeshComponent* meshComponent = gameObject->AddComponent<WaterLavaMeshComponent>();
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

    WaterLavaMeshComponent* meshComponent = gameObject->AddComponent<WaterLavaMeshComponent>();
    meshComponent->SetWaterLavaTiles(tilesArray);
    meshComponent->SetSurfaceTexture(gTexturesManager.mWaterTexture);
    meshComponent->SetSurfaceParams(DEFAULT_WATER_TRANSLUCENCY, DEFAULT_WATER_WAVE_WIDTH, DEFAULT_WATER_WAVE_HEIGHT, DEFAULT_WATER_WAVE_FREQ, DEFAULT_WATER_LEVEL);

    return gameObject;
}

void TerrainManager::HighhlightTile(TerrainTile* mapTile, bool isHighlighted)
{
    if (mapTile && mapTile->mIsTagged != isHighlighted)
    {
        cxx::push_back_if_unique(mHighlightTiles, mapTile);
        mapTile->mIsTagged = isHighlighted;
    }
    debug_assert(mapTile);
}