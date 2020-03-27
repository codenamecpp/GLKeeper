#include "pch.h"
#include "GameWorld.h"
#include "ScenarioLoader.h"
#include "Console.h"
#include "TerrainManager.h"
#include "RoomsManager.h"

GameWorld gGameWorld;

bool GameWorld::InitializeWorld(const std::string& scenarioName)
{
    ScenarioLoader scenarioLoader (mScenarioData);
    if (!scenarioLoader.LoadScenarioData(scenarioName))
    {
        Deinit();

        gConsole.LogMessage(eLogMessage_Warning, "Cannot load scenario data");
        return false;
    }

    SetupMapData(0xDEADBEEF);

    if (!gTerrainManager.Initialize())
    {
        Deinit();

        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize terrain manager");
        return false;
    }

    if (!gRoomsManager.Initialize())
    {
        Deinit();
        
        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize rooms manager");
        return false;
    }

    ConstructStartupRooms();
    return true;
}

void GameWorld::Deinit()
{
    gRoomsManager.Deinit();
    gTerrainManager.Deinit();
    mScenarioData.Clear();
    mMapData.Clear();
}

void GameWorld::UpdateFrame()
{
    gRoomsManager.UpdateFrame();
    gTerrainManager.UpdateTerrainMesh();
}

void GameWorld::SetupMapData(unsigned int seed)
{
    Size2D mapDimensions (mScenarioData.mLevelDimensionX, mScenarioData.mLevelDimensionY);
    mMapData.Setup(mapDimensions, seed);

    int currentTileIndex = 0;
    for (int tiley = 0; tiley < mScenarioData.mLevelDimensionY; ++tiley)
    for (int tilex = 0; tilex < mScenarioData.mLevelDimensionX; ++tilex)
    {
        MapTile* currentTile = mMapData.GetMapTile(Point2D(tilex, tiley));
        debug_assert(currentTile);

        TerrainTypeID tileTerrainType = mScenarioData.mMapTiles[currentTileIndex].mTerrainType;
        if (IsRoomTypeTerrain(tileTerrainType))
        {
            RoomDefinition* roomDefinition = GetRoomDefinitionByTerrain(tileTerrainType);
            // acquire terrain type under the bridge
            if (!roomDefinition->mPlaceableOnLand)
            {
                switch (mScenarioData.mMapTiles[currentTileIndex].mTerrainUnderTheBridge)
                {
                    case eBridgeTerrain_Lava:
                        currentTile->mBaseTerrain = GetLavaTerrain();
                    break;

                    case eBridgeTerrain_Water:
                        currentTile->mBaseTerrain = GetWaterTerrain();
                    break;
                }
            }
            else
            {
                // claimed path is default
                currentTile->mBaseTerrain = GetPlayerColouredPathTerrain();
            }
            // set room terrain type
            currentTile->mRoomTerrain = GetTerrainDefinition(tileTerrainType);
        }
        else
        {
            currentTile->mBaseTerrain = GetTerrainDefinition(tileTerrainType);
        }

        debug_assert(currentTile->mBaseTerrain);
        // set additional tile params
        currentTile->mOwnerId = mScenarioData.mMapTiles[currentTileIndex].mOwnerIdentifier;

        ++currentTileIndex;
    }
}

void GameWorld::ConstructStartupRooms()
{
    // create rooms
    for (int tiley = 0; tiley < mMapData.mDimensions.y; ++tiley)
    for (int tilex = 0; tilex < mMapData.mDimensions.x; ++tilex)
    {
        const Point2D currTileLocation (tilex, tiley);

        MapTile* currTile = mMapData.GetMapTile(currTileLocation);
        if (currTile->mRoom)
        {
            // room is already constructed on this tile
            continue;
        }

        TerrainDefinition* terrainDef = currTile->GetTerrain();
        if (IsRoomTypeTerrain(terrainDef))
        {
            ConstructStartupRoom(currTile);
        }
    }
}

void GameWorld::ConstructStartupRoom(MapTile* initialTile)
{
    debug_assert(initialTile);

    TerrainDefinition* terrainDef = initialTile->GetTerrain();
    // query room definition
    RoomDefinition* roomDefinition = GetRoomDefinitionByTerrain(terrainDef);
    if (roomDefinition == nullptr)
    {
        debug_assert(false);
        return;
    }

    TilesArray floodTiles;

    MapFloodFillFlags floodFillFlags;
    floodFillFlags.mSameBaseTerrain = false;
    floodFillFlags.mSameOwner = true;
    mMapData.FloodFill4(floodTiles, initialTile,floodFillFlags);

    // create room instance
    GenericRoom* roomInstance = gRoomsManager.CreateRoomInstance(roomDefinition, initialTile->mOwnerId, floodTiles);
    debug_assert(roomInstance);
}

TerrainDefinition* GameWorld::GetLavaTerrain()
{
    debug_assert(mScenarioData.mLavaTerrainType != TerrainType_Null);
    return GetTerrainDefinition(mScenarioData.mLavaTerrainType);
}

TerrainDefinition* GameWorld::GetWaterTerrain()
{
    debug_assert(mScenarioData.mWaterTerrainType != TerrainType_Null);
    return GetTerrainDefinition(mScenarioData.mWaterTerrainType);
}

TerrainDefinition* GameWorld::GetPlayerColouredPathTerrain()
{
    debug_assert(mScenarioData.mPlayerColouredPathTerrainType != TerrainType_Null);
    return GetTerrainDefinition(mScenarioData.mPlayerColouredPathTerrainType);
}

TerrainDefinition* GameWorld::GetPlayerColouredWallTerrain()
{
    debug_assert(mScenarioData.mPlayerColouredWallTerrainType != TerrainType_Null);
    return GetTerrainDefinition(mScenarioData.mPlayerColouredWallTerrainType);
}

TerrainDefinition* GameWorld::GetFogOfWarTerrain()
{
    debug_assert(mScenarioData.mFogOfWarTerrainType != TerrainType_Null);
    return GetTerrainDefinition(mScenarioData.mFogOfWarTerrainType);
}

TerrainDefinition* GameWorld::GetTerrainDefinition(const std::string& typeName)
{
    for (TerrainDefinition& currentDefinition: mScenarioData.mTerrainDefs)
    {
        if (currentDefinition.mName == typeName)
            return &currentDefinition;
    }
    return nullptr;
}

TerrainDefinition* GameWorld::GetTerrainDefinition(TerrainTypeID typeID)
{
    debug_assert(typeID < mScenarioData.mTerrainDefs.size());
    return &mScenarioData.mTerrainDefs[typeID];
}

GameObjectDefinition* GameWorld::GetGameObjectDefinition(const std::string& typeName)
{
    for (GameObjectDefinition& currentDefinition: mScenarioData.mGameObjectDefs)
    {
        if (currentDefinition.mObjectName == typeName)
            return &currentDefinition;
    }
    return nullptr;
}

GameObjectDefinition* GameWorld::GetGameObjectDefinition(GameObjectTypeID typeID)
{
    debug_assert(typeID < mScenarioData.mGameObjectDefs.size());
    return &mScenarioData.mGameObjectDefs[typeID];
}

RoomDefinition* GameWorld::GetRoomDefinition(const std::string& typeName)
{
    for (RoomDefinition& currentDefinition: mScenarioData.mRoomDefs)
    {
        if (currentDefinition.mRoomName == typeName)
            return &currentDefinition;
    }
    return nullptr;
}

RoomDefinition* GameWorld::GetRoomDefinition(RoomTypeID typeID)
{
    debug_assert(typeID < mScenarioData.mRoomDefs.size());
    return &mScenarioData.mRoomDefs[typeID];
}

CreatureDefinition* GameWorld::GetCreatureDefinition(const std::string& typeName)
{
    for (CreatureDefinition& currentDefinition: mScenarioData.mCreatureDefs)
    {
        if (currentDefinition.mCreatureName == typeName)
            return &currentDefinition;
    }
    return nullptr;
}

CreatureDefinition* GameWorld::GetCreatureDefinition(CreatureTypeID typeID)
{
    debug_assert(typeID < mScenarioData.mCreatureDefs.size());
    return &mScenarioData.mCreatureDefs[typeID];
}

RoomDefinition* GameWorld::GetRoomDefinitionByTerrain(TerrainDefinition* terrainDefinition)
{
    if (terrainDefinition == nullptr)
    {
        debug_assert(false);
        return &mScenarioData.mRoomDefs[RoomType_Null];
    }

    return GetRoomDefinitionByTerrain(terrainDefinition->mTerrainType);
}

RoomDefinition* GameWorld::GetRoomDefinitionByTerrain(TerrainTypeID typeID)
{
    return GetRoomDefinition(mScenarioData.mRoomByTerrainType[typeID]);
}

bool GameWorld::IsRoomTypeTerrain(TerrainDefinition* terrainDefinition) const
{
    if (terrainDefinition == nullptr)
    {
        debug_assert(false);
        return false;
    }

    return IsRoomTypeTerrain(terrainDefinition->mTerrainType);
}

bool GameWorld::IsRoomTypeTerrain(TerrainTypeID typeID) const
{
    return mScenarioData.mRoomByTerrainType[typeID] != RoomType_Null;
}
