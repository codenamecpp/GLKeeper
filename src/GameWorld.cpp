#include "stdafx.h"
#include "GameWorld.h"
#include "EnvironmentMeshObject.h"
#include "GameRenderManager.h"
#include "AnimatingMeshObject.h"
#include "RoomManager.h"
#include "Room.h"
#include "GameObject.h"
#include "GameSession.h"
#include "CreatureManager.h"
#include "Physics.h"
#include "GameObjectManager.h"
#include "NavigationService.h"
#include "Scene.h"
#include "GameMap.h"
#include "CreatureTaskManager.h"

//////////////////////////////////////////////////////////////////////////

GameWorld gGameWorld;

//////////////////////////////////////////////////////////////////////////

bool GameWorld::LoadScenario(const ScenarioDefinition& scenarioDefinition, GameLoadingAware& loadingContext)
{
    gScene.Initialize();

    loadingContext.UpdateLoadingProgress(0.0f);

    // configure game time
    gTime.ResetFixedClock(eFixedClock::GameLogic);
    gTime.SetFixedClockFramerate(eFixedClock::GameLogic, scenarioDefinition.mTicksPerSecond);

    mTileConstructionSet.Initialize(scenarioDefinition);

    loadingContext.UpdateLoadingProgress(0.1f);

    gPhysics.LoadScenario(scenarioDefinition);
    gRoomManager.LoadScenario(scenarioDefinition);

    loadingContext.UpdateLoadingProgress(0.15f);

    gGameMap.LoadScenario(scenarioDefinition);

    loadingContext.UpdateLoadingProgress(0.2f);

    { // create rooms
        GameMap::TilesIterator tilesIterator = gGameMap.IterateTiles();
        for (MapTile* mapTile = tilesIterator.NextTile(); mapTile; 
            mapTile = tilesIterator.NextTile())
        {
            if (mapTile->mRoomInstance)
                continue;

            if (scenarioDefinition.IsRoomTypeTerrain(mapTile->GetTerrain()))
            {
                CreateRoomFromUnexploredTiles(mapTile, scenarioDefinition);
            }
        }
    }

    loadingContext.UpdateLoadingProgress(0.3f);

    { // construct base terrain, water bed
        TileConstructor& tileConstructor = *mTileConstructionSet.GetBaseConstructor();

        GameMap::TilesIterator tilesIterator = gGameMap.IterateTiles();
        for (MapTile* mapTile = tilesIterator.NextTile(); mapTile; 
            mapTile = tilesIterator.NextTile())
        {
            tileConstructor.ConstructTile(mapTile);
        }
    }

    loadingContext.UpdateLoadingProgress(0.4f);

    CreateEnvironmentEntities();

    loadingContext.UpdateLoadingProgress(0.5f);

    // build room geometry
    gRoomManager.ProcessRoomChanges();
    for (Room* roomInstance: gRoomManager.GetRooms())
    {
        RoomTileConstructor* roomConstructor = roomInstance->GetTileConstructor();
        // floor
        roomConstructor->ConstructRoomFloor(roomInstance, roomInstance->GetFloorTiles());

        // walls
        for (const RoomWallSection* wallSection: roomInstance->GetWallSections())
        {
            roomConstructor->ConstructRoomWalls(roomInstance, wallSection->GetTiles(), wallSection->GetFace());
        }
    }

    loadingContext.UpdateLoadingProgress(0.6f);

    gGameRenderer.mTerrainRenderer.CreateTerrainMesh();

    loadingContext.UpdateLoadingProgress(0.7f);

    ResetInvalidatedTiles();
    InitTilesFloorHeightmap();

    loadingContext.UpdateLoadingProgress(0.8f);

    gGameObjectManager.LoadScenario(scenarioDefinition);

    loadingContext.UpdateLoadingProgress(0.9f);

    gCreatureManager.LoadScenario(scenarioDefinition);
    gCreatureTaskManager.LoadScenario(scenarioDefinition);

    mTileSelectionOutline.Init(gScene);

    gScene.GetCamera().ResetOrientation();

    EnterWorld();

    loadingContext.UpdateLoadingProgress(1.0f);
    return true;
}

void GameWorld::EnterWorld()
{
    gPhysics.EnterWorld();
    gNavigationService.EnterWorld();
    gRoomManager.EnterWorld();
    gGameObjectManager.EnterWorld();
    gCreatureTaskManager.EnterWorld();
    gCreatureManager.EnterWorld();

    mCurrentFrameStats.Clear();
    mPrevFrameStats.Clear();
}

void GameWorld::ClearWorld()
{   
    mEnvironmentObjects.clear();
    mTileSelectionOutline.Deinit();
    gRoomManager.ClearWorld();
    gCreatureManager.ClearWorld();
    gCreatureTaskManager.ClearWorld();
    gGameObjectManager.ClearWorld();
    gNavigationService.ClearWorld();
    gPhysics.ClearWorld();
    gGameRenderer.mTerrainRenderer.CleanupTerrainMesh();
    gGameMap.Cleanup();
    gScene.ClearScene();
    mTileConstructionSet.Cleanup();
    mNextEntityUid = 1;

    gScene.Shutdown();
}

void GameWorld::UpdateFrame(float deltaTime)
{
    mPrevFrameStats = mCurrentFrameStats;

    mCurrentFrameStats.Clear();
    mCurrentFrameStats.mNumSceneObjectsActive = gScene.GetActiveSceneObjectCount();

    mTileSelectionOutline.UpdateFrame();

    BuildInvalidatedTiles();

    // tick those subsystems after refresh terrain geometries
    gGameObjectManager.UpdateFrame(deltaTime);
    gCreatureManager.UpdateFrame(deltaTime);
    gRoomManager.UpdateFrame(deltaTime);
    gNavigationService.UpdateFrame(deltaTime);
    gCreatureTaskManager.UpdateFrame(deltaTime);
    gPhysics.UpdateFrame(deltaTime);
    gScene.UpdateFrame(deltaTime);
}

void GameWorld::UpdateLogic(float stepDeltaTime)
{
    gGameObjectManager.UpdateLogic(stepDeltaTime);
    gRoomManager.UpdateLogic(stepDeltaTime);
    gCreatureManager.UpdateLogic(stepDeltaTime);
    gCreatureTaskManager.UpdateLogic(stepDeltaTime);
    gNavigationService.UpdateLogic(stepDeltaTime);
}

void GameWorld::UpdatePhysics(float stepDeltaTime)
{
    gCreatureManager.UpdatePhysics(stepDeltaTime);
    gGameObjectManager.UpdatePhysics(stepDeltaTime);
    gPhysics.UpdatePhysics(stepDeltaTime);
}

bool GameWorld::CastRayFromScreenPoint(const Point2D& screenCoordinate, cxx::ray3d_t& resultRay)
{
    const Viewport& viewport = gRenderDevice.GetViewport();

    Camera& sceneCamera = gScene.GetCamera();
    sceneCamera.ComputeMatricesAndFrustum(gRenderDevice.GetViewport());

    // wrap y
    const int mouseY = viewport.mScreenArea.h - screenCoordinate.y;

    glm::ivec4 vp ( viewport.mScreenArea.x, viewport.mScreenArea.y, viewport.mScreenArea.w, viewport.mScreenArea.h );
    //unproject twice to build a ray from near to far plane
    const glm::vec3 v0 = glm::unProject(glm::vec3{screenCoordinate.x * 1.0f, mouseY * 1.0f, 0.0f}, 
        sceneCamera.mViewMatrix, 
        sceneCamera.mProjectionMatrix, vp); // near plane

    const glm::vec3 v1 = glm::unProject(glm::vec3{screenCoordinate.x * 1.0f, mouseY * 1.0f, 1.0f}, 
        sceneCamera.mViewMatrix, 
        sceneCamera.mProjectionMatrix, vp); // far plane

    resultRay.mOrigin = v0;
    resultRay.mDirection = glm::normalize(v1 - v0);
    return true;
}

void GameWorld::InvalidateTile(MapTile* mapTile)
{
    cxx_assert(mapTile);

    // set invalidated flag for all faces
    for (TileFaceData& faces: mapTile->mFaces)
    {
        faces.mFaceMeshDirty = true;
    }

    // is not already queued
    if (!cxx::contains(mInvalidatedTiles, mapTile))
    {
        mInvalidatedTiles.push_back(mapTile);
    }
}

void GameWorld::InvalidateTile(MapTile* mapTile, eTileFace face)
{
    cxx_assert(mapTile);

    mapTile->mFaces[face].mFaceMeshDirty = true;

    // is not already queued
    if (!cxx::contains(mInvalidatedTiles, mapTile))
    {
        mInvalidatedTiles.push_back(mapTile);
    }
}

void GameWorld::InvalidateTiles(cxx::span<MapTile*> mapTiles)
{
    for (MapTile* roller: mapTiles)
    {
        InvalidateTile(roller);
    }
}

void GameWorld::InvalidateTileNeighbours(MapTile* mapTile)
{
    cxx_assert(mapTile);
    for (MapTile* neighbourTile: mapTile->mNeighbours)
    {
        if (neighbourTile)
        {
            InvalidateTile(neighbourTile);
        }
    }
}

EntityUid GameWorld::GenerateEntityUid()
{
    return mNextEntityUid++;
}

void GameWorld::CreateEnvironmentEntities()
{
    Temp_Vector<MapTile*> lavaTiles;
    Temp_Vector<MapTile*> waterTiles;
    Temp_Vector<MapTile*> tempTiles;

    lavaTiles.reserve(128);
    waterTiles.reserve(128);

    // find water and lava tiles
    GameMap::TilesIterator tilesIterator = gGameMap.IterateTiles();
    for (MapTile* currMapTile = tilesIterator.NextTile(); currMapTile; currMapTile = tilesIterator.NextTile())
    {
        const TerrainDefinition* baseTerrainDef = currMapTile->GetBaseTerrain();
        // collect lava tile
        if (baseTerrainDef->mIsLava)
        {
            lavaTiles.push_back(currMapTile);
        }
        // collect water tile
        if (baseTerrainDef->mIsWater)
        {
            waterTiles.push_back(currMapTile);
        }
    }

    // create water and lava surfaces
    while (!lavaTiles.empty())
    {
        MapTile* originTile = lavaTiles.back();
        lavaTiles.pop_back();

        tempTiles.clear();
        gGameMap.FloodFill4(tempTiles, originTile, FLOOD_FILL4_SAME_BASE_TERRAIN);
        if (tempTiles.empty())
        {
            cxx_assert(false);
            continue;
        }

        cxx::uniqueptr<EnvironmentMeshObject> lavaSurface = gScene.CreateLavaMesh(tempTiles);
        cxx_assert(lavaSurface);
        if (lavaSurface)
        {
            lavaSurface->SetObjectActive(true);
            mEnvironmentObjects.push_back(std::move(lavaSurface));
        }
        else
        {
            gConsole.LogMessage(eLogLevel_Warning, "Cannot create level lava surface");
        }
        // remove used tiles
        cxx::erase_elements(lavaTiles, tempTiles);
    }

    while (!waterTiles.empty())
    {
        MapTile* originTile = waterTiles.back();
        waterTiles.pop_back();

        tempTiles.clear();
        gGameMap.FloodFill4(tempTiles, originTile, FLOOD_FILL4_SAME_BASE_TERRAIN);
        if (tempTiles.empty())
        {
            cxx_assert(false);
            continue;
        }

        cxx::uniqueptr<EnvironmentMeshObject> waterSurface = gScene.CreateWaterMesh(tempTiles);
        cxx_assert(waterSurface);
        if (waterSurface)
        {
            waterSurface->SetObjectActive(true);
            mEnvironmentObjects.push_back(std::move(waterSurface));
        }
        else
        {
            gConsole.LogMessage(eLogLevel_Warning, "Cannot create level water surface");
        }
        // remove used tiles
        cxx::erase_elements(waterTiles, tempTiles);
    }
}

void GameWorld::CreateRoomFromUnexploredTiles(MapTile* startTile, const ScenarioDefinition& scenarioDefinition)
{
    cxx_assert(startTile);

    // query room definition
    RoomDefinition* roomDefinition = gGameSession.GetScenarioDefinition().GetRoomDefinitionByTerrain(startTile->GetTerrain());
    cxx_assert(roomDefinition);
    if (!roomDefinition)
        return;

    Temp_Vector<MapTile*> floodTiles;
    gGameMap.FloodFill4(floodTiles, startTile);

    // get room extents
    MapPoint2D minPoint = startTile->mLocation;
    MapPoint2D maxPoint = startTile->mLocation;
    for (MapTile* roller: floodTiles)
    {
        minPoint = glm::min(minPoint, roller->mLocation);
        maxPoint = glm::max(maxPoint, roller->mLocation);
    }
    MapPoint2D roomCenter = (minPoint + maxPoint) / 2;
    // check for init params
    int initParamsIdx = cxx::get_first_index_if(scenarioDefinition.mRoomThings, 
        [&roomCenter, roomDefinition](const ScenarioRoomThing& initParams)
        {
            return (initParams.mRoomType == roomDefinition->mRoomType) && 
                (initParams.mPositionX == roomCenter.x) && 
                (initParams.mPositionY == roomCenter.y);
        });

    const ScenarioRoomThing* roomInitParams = nullptr;
    if (initParamsIdx != -1)
    {
        roomInitParams = &scenarioDefinition.mRoomThings[initParamsIdx];
    }

    const ePlayerID ownerId = roomInitParams ? roomInitParams->mPlayerId : startTile->mOwnerId;

    // create room instance
    EntityHandle roomHandle = roomInitParams ?
        gRoomManager.CreateScenarioRoom(*roomInitParams) :
        gRoomManager.CreateRoom(roomDefinition, ownerId);

    gGameSession.GetPlayer(ownerId).AddToInventory(roomHandle);
    gRoomManager.ActivateRoom(roomHandle);
    if (Room* roomInstance = gRoomManager.GetRoomPtr(roomHandle))
    {
        roomInstance->EnlargeRoom(floodTiles);
    }
}

bool GameWorld::DigTile(MapTile* mapTile, ePlayerID playerId, long& outGoldMined)
{
    cxx_assert(mapTile);

    if (mapTile == nullptr)
        return false;

    TerrainDefinition* terrainDef = mapTile->GetTerrain();
    if (!terrainDef->mIsSolid)
        return false;

    const ScenarioVariables& scenarioVars = gGameSession.GetScenarioVariables();

    outGoldMined = 0;

    const bool isTileImpenetrable = terrainDef->mIsImpenetrable;
    // mining gold?
    if (mapTile->IsMoneySource())
    {
        // gems ?
        if (isTileImpenetrable)
        {
            outGoldMined = scenarioVars.mGoldMinedFromGems;
        }
        else // regular gold
        {
            const int deltaHitPoints = ChangeTileHealth(mapTile, playerId, scenarioVars.mMineGoldHealth);
            if (deltaHitPoints > 0)
            {
                cxx_assert(terrainDef->mHealthMax);
                outGoldMined = static_cast<int>(((deltaHitPoints * 1.0f) / (terrainDef->mHealthMax * 1.0f)) * terrainDef->mGoldValue + 0.5f);
            }
            cxx_assert(deltaHitPoints > 0);
        }
        return outGoldMined > 0;
    }

    if (!isTileImpenetrable)
    {
        outGoldMined = 0;

        int digHealth = scenarioVars.mDigRockHealth;
        // reinforced wall?
        if (terrainDef->mIsOwnable)
        {
            digHealth = mapTile->HasOwner(playerId) ? 
                scenarioVars.mDigOwnWallHealth : 
                scenarioVars.mDigEnemyWallHealth;
        }

        const int deltaHitPoints = ChangeTileHealth(mapTile, playerId, digHealth);
        if (deltaHitPoints > 0)
            return true;

        cxx_assert(false);
    }
    return false;
}

bool GameWorld::ReinforceWall(MapTile* mapTile, ePlayerID playerId, bool& outCompleted)
{
    outCompleted = false;

    cxx_assert(mapTile);
    if (mapTile == nullptr)
        return false;

    TerrainDefinition* terrainDef = mapTile->GetTerrain();
    if (!terrainDef->mIsSolid || terrainDef->mIsOwnable || terrainDef->mIsImpenetrable)
        return false;

    const TerrainTypeId nextTerrainTypeId = terrainDef->mBecomesTerrainTypeWhenMaxHealth;
    if ((nextTerrainTypeId == TerrainTypeId_Null) || 
        (nextTerrainTypeId == terrainDef->mTerrainType))
    {
        return false;
    }

    const TerrainDefinition* nextTerrainDef = gGameSession.GetScenarioDefinition().GetTerrainDefinition(nextTerrainTypeId);
    cxx_assert(nextTerrainDef);
    if (!nextTerrainDef->mIsSolid || !nextTerrainDef->mIsOwnable)
        return false;

    const ScenarioVariables& scenarioVars = gGameSession.GetScenarioVariables();

    const int deltaHitPoints = ChangeTileHealth(mapTile, playerId, scenarioVars.mReinforceWallHealth);
    // reinforced?
    if (mapTile->GetTerrain() == nextTerrainDef)
    {
        outCompleted = true;
        return true;
    }
    return (deltaHitPoints != 0);
}

bool GameWorld::ClaimFloor(MapTile* mapTile, ePlayerID playerId, bool& outCompleted)
{
    outCompleted = false;

    cxx_assert(mapTile);
    if (mapTile == nullptr)
        return false;

    TerrainDefinition* terrainDef = mapTile->GetTerrain();
    if (terrainDef->mIsSolid || terrainDef->mIsOwnable)
        return false;

    const TerrainTypeId nextTerrainTypeId = terrainDef->mBecomesTerrainTypeWhenMaxHealth;
    if ((nextTerrainTypeId == TerrainTypeId_Null) || 
        (nextTerrainTypeId == terrainDef->mTerrainType))
    {
        return false;
    }

    const TerrainDefinition* nextTerrainDef = gGameSession.GetScenarioDefinition().GetTerrainDefinition(nextTerrainTypeId);
    cxx_assert(nextTerrainDef);
    if (nextTerrainDef->mIsSolid || !nextTerrainDef->mIsOwnable)
        return false;

    // must be adjacent to owned territory
    bool bordersWithOwnedTerritory = false;
    for (eDirection dir: gStraightDirections)
    {
        const MapTile* neighbourTile = mapTile->mNeighbours[dir];
        if (neighbourTile == nullptr)
            continue;

        const TerrainDefinition* neighbourTileDef = neighbourTile->GetTerrain();
        if (neighbourTileDef->mIsOwnable && neighbourTile->HasOwner(playerId))
        {
            bordersWithOwnedTerritory = true;
            break;
        }
    }

    if (!bordersWithOwnedTerritory)
        return false;

    const ScenarioVariables& scenarioVars = gGameSession.GetScenarioVariables();
    const int deltaHitPoints = ChangeTileHealth(mapTile, playerId, scenarioVars.mClaimFloorHealth);
    // reinforced?
    if (mapTile->GetTerrain() == nextTerrainDef)
    {
        outCompleted = true;
        return true;
    }
    return (deltaHitPoints != 0);
}

void GameWorld::DamageTile(MapTile* mapTile, ePlayerID playerId, int hitPoints)
{
    cxx_assert(hitPoints > 0);
    ChangeTileHealth(mapTile, playerId, -hitPoints);
}

void GameWorld::RepairTile(MapTile* mapTile, ePlayerID playerId, int hitPoints)
{
    cxx_assert(hitPoints > 0);
    ChangeTileHealth(mapTile, playerId, hitPoints);
}

void GameWorld::TagTilesForDigging(const MapArea2D& tileArea, ePlayerID playerId, bool setTagged)
{
    auto tilesIterator = gGameMap.IterateTiles(tileArea);
    for (MapTile* mapTile = tilesIterator.NextTile(); mapTile; 
        mapTile = tilesIterator.NextTile())
    {
        // set tag
        if (setTagged)
        {
            if (CanTagTileForDigging(mapTile))
            {
                UpdateTileTaggedState(mapTile, playerId, true);
            }
        }
        // clear tag
        else if (mapTile->IsTaggedForDigging(playerId))
        {
            UpdateTileTaggedState(mapTile, playerId, false);
        }
    }
}

bool GameWorld::CanPlaceRoomOnLocation(MapTile* mapTile, ePlayerID playerId, RoomDefinition* roomDefinition) const
{
    cxx_assert(mapTile);

    TerrainDefinition* tileTerrain = mapTile->GetTerrain();
    if (gGameSession.GetScenarioDefinition().IsRoomTypeTerrain(tileTerrain))
        return false;

    // cannot place on mana vault
    if (tileTerrain->mTerrainType == TerrainTypeId_ClaimedVault) 
        return false;

    if (roomDefinition->mPlaceableOnLand)
    {
        if (!tileTerrain->mIsSolid && tileTerrain->mIsOwnable && mapTile->HasOwner(playerId))
            return true;
    }

    if (roomDefinition->mPlaceableOnLava && tileTerrain->mIsLava)
        return true;

    if (roomDefinition->mPlaceableOnWater && tileTerrain->mIsWater)
        return true;

    return false;
}

bool GameWorld::CanSellRoomOnLocation(MapTile* mapTile, ePlayerID playerId) const
{
    cxx_assert(mapTile);

    Room* roomInstance = mapTile->mRoomInstance;
    if (!roomInstance || (playerId != mapTile->mOwnerId))
        return false;

    RoomDefinition* roomDefinition = roomInstance->GetDefinition();
    return (roomDefinition->mBuildable == true);
}

void GameWorld::SellEntities(ePlayerID playerId, const MapArea2D& tilesArea)
{
    Temp_Vector<MapTile*> roomTiles;
    roomTiles.reserve(64);

    auto tilesIterator = gGameMap.IterateTiles(tilesArea);
    for (MapTile* mapTile = tilesIterator.NextTile(); mapTile; 
        mapTile = tilesIterator.NextTile())
    {
        if (CanSellRoomOnLocation(mapTile, playerId))
        {
            roomTiles.push_back(mapTile);
        }
    }

    if (roomTiles.empty()) return;

    // sort by room instance
    std::sort(roomTiles.begin(), roomTiles.end(), [](MapTile* lhs, MapTile* rhs) 
        { 
            return lhs->mRoomInstance < rhs->mRoomInstance; 
        });

    // helper
    auto countRoomTiles = [](cxx::span<MapTile*> tiles)
        {
            int counter = 0;
            Room* roomInstance = tiles[0]->mRoomInstance;
            for (MapTile* roller: tiles) 
            {
                if (roller->mRoomInstance != roomInstance) break; 
                ++counter; 
            }
            return counter;
        };

    for (int itile = 0, NumTiles = roomTiles.size(); itile < NumTiles; )
    {
        Room* roomInstance = roomTiles[itile]->mRoomInstance;
        // count tiles of same room
        int numReleaseRoomTiles = countRoomTiles({
            roomTiles.data() + itile, 
            roomTiles.data() + NumTiles});
        ReleaseRoomTiles(roomInstance, {
            roomTiles.data() + itile, 
            roomTiles.data() + itile + numReleaseRoomTiles});
        itile += numReleaseRoomTiles;
    }
}

void GameWorld::ConstructRoom(ePlayerID playerId, RoomDefinition* roomDefinition, const MapArea2D& tilesArea)
{
    // collect all tiles available to construction, output array is sorted

    Temp_Vector<MapTile*> constructionTiles;
    constructionTiles.reserve(tilesArea.w * tilesArea.h);

    auto tilesIterator = gGameMap.IterateTiles(tilesArea);
    for (MapTile* mapTile = tilesIterator.NextTile(); mapTile; 
        mapTile = tilesIterator.NextTile())
    {
        if (CanPlaceRoomOnLocation(mapTile, playerId, roomDefinition))
        {
            constructionTiles.push_back(mapTile);
        }
    }
    
    if (constructionTiles.empty()) return; // nothing to construct

    Temp_Set<MapTile*> processedTiles;
    Temp_List<Room*> adjacentRooms;

    Temp_Vector<MapTile*> segmentTiles;

    // scan for contiguous segments
    for (MapTile* currentTile: constructionTiles)
    {
        // is processed already ?
        if (cxx::contains(processedTiles, currentTile))
            continue;

        segmentTiles.clear();
        gGameMap.FloodFill4(segmentTiles, currentTile, tilesArea);
        // put into processed set
        for (MapTile* processedTile: segmentTiles)
        {
            processedTiles.insert(processedTile);
        }

        // find rooms that contacting with current segment and merge them all

        adjacentRooms.clear();
        ScanAdjacentRooms(segmentTiles, playerId, 
            [roomDefinition](Room* checkRoom) { return checkRoom->GetDefinition() == roomDefinition; }, adjacentRooms);

        Room* absorberRoom = nullptr;

        // get largest room absorber
        if (!adjacentRooms.empty())
        {
            absorberRoom = adjacentRooms.front();
            for (Room* roller: adjacentRooms)
            {
                if (roller->GetRoomSize() > absorberRoom->GetRoomSize())
                {
                    absorberRoom = roller;
                }
            }
            cxx_assert(absorberRoom);
        }

        if (absorberRoom == nullptr) // new room
        {
            EntityHandle roomHandle = gRoomManager.CreateRoom(roomDefinition, playerId);
            gGameSession.GetPlayer(playerId).AddToInventory(roomHandle);
            gRoomManager.ActivateRoom(roomHandle);
            absorberRoom = gRoomManager.GetRoomPtr(roomHandle);
        }

        // add segment tiles to room
        for (MapTile* processedTile: segmentTiles)
        {
            cxx_assert(processedTile->mRoomInstance == nullptr);
            processedTile->SetRoomTerrain(gGameSession.GetScenarioDefinition().GetTerrainDefinition(roomDefinition->mTerrainType));
            processedTile->mOwnerId = playerId;
            gCreatureTaskManager.OnTileTerrainTypeChanged(processedTile);
        }
        absorberRoom->EnlargeRoom(segmentTiles);

        // absorb adjacent rooms
        for (Room* adjacentRoom: adjacentRooms)
        {
            if (adjacentRoom == absorberRoom) continue;

            absorberRoom->AbsorbRoom(adjacentRoom);
            HandleRoomAbsorbed(adjacentRoom);
        }
    }
}

void GameWorld::ReleaseRoomTiles(Room* currentRoom, cxx::span<MapTile*> roomTiles)
{
    cxx_assert(currentRoom);
    currentRoom->ReleaseTiles(roomTiles);

    // change terrain
    for (MapTile* roomTile: roomTiles)
    {
        RoomDefinition* roomDefinition = currentRoom->GetDefinition();
        roomTile->SetRoomTerrain(nullptr);
        if (!roomDefinition->mPlaceableOnLand && 
            (roomDefinition->mPlaceableOnLava || roomDefinition->mPlaceableOnWater))
        {   
            roomTile->mOwnerId = ePlayerID_Neutral; // for bridges terrain reset owner
        }
        gCreatureTaskManager.OnTileTerrainTypeChanged(roomTile);
    }

    int segmentsCounter = 0;
    EnumRoomSegments(currentRoom, [this, &segmentsCounter, currentRoom](cxx::span<MapTile*> segmentTiles)
    {
        if (segmentsCounter++ == 0) // first segment is current room segment
            return;

        ePlayerID ownerId = currentRoom->GetOwnerId();

        // each new segment is a new room
        EntityHandle newRoom = gRoomManager.CreateRoom(currentRoom->GetDefinition(), ownerId);
        gGameSession.GetPlayer(ownerId).AddToInventory(newRoom);
        gRoomManager.ActivateRoom(newRoom);
        if (Room* roomInstance = gRoomManager.GetRoomPtr(newRoom))
        {
            roomInstance->AbsorbRoom(currentRoom, segmentTiles);
        }
    });

    // empty room collapses
    if (currentRoom->GetRoomSize() == 0)
    {
        HandleRoomCollapsed(currentRoom);
    }
}

void GameWorld::HandleRoomAbsorbed(Room* roomInstance)
{
    cxx_assert(roomInstance);
    if (roomInstance == nullptr) return;
    if (roomInstance->GetRoomSize() > 0)
    {
        cxx_assert(false);
    }
    // delete it
    ePlayerID ownerId = roomInstance->GetOwnerId();
    gGameSession.GetPlayer(ownerId).RemoveFromInventory(roomInstance->GetOwnHandle());
    gRoomManager.DeleteRoom(roomInstance->GetInstanceUid());
}

void GameWorld::HandleRoomCollapsed(Room* roomInstance)
{
    cxx_assert(roomInstance);
    if (roomInstance == nullptr) return;
    if (roomInstance->GetRoomSize() > 0)
    {
        cxx_assert(false);
    }
    // delete it
    ePlayerID ownerId = roomInstance->GetOwnerId();
    gGameSession.GetPlayer(ownerId).RemoveFromInventory(roomInstance->GetOwnHandle());
    gRoomManager.DeleteRoom(roomInstance->GetInstanceUid());
}

void GameWorld::BuildInvalidatedTiles()
{
    if (mInvalidatedTiles.empty())
        return;

    Temp_Set<Room*> invalidateRooms;

    // build terrain tiles and collect invalidated rooms
    
    TileConstructor& getTileConstructor = *GetTileConstructionSet().GetBaseConstructor();
    for (MapTile* currentTile : mInvalidatedTiles)
    {
        gGameRenderer.mTerrainRenderer.InvalidateTile(currentTile);

        // traverse each invalidated tile face
        for (eTileFace iface: gTileFaces)
        {
            if (!currentTile->mFaces[iface].mFaceMeshDirty) // rebuild only invalidated face of tile
                continue;

            getTileConstructor.ConstructTile(currentTile, iface);

            // room should rebuild its wall mesh
            if (currentTile->mFaces[iface].mWallExtendsRoom)
            {
                eDirection faceDirection = TileFaceToDirection(iface);
                Room* roomInstance = currentTile->mNeighbours[faceDirection]->mRoomInstance;
                cxx_assert(roomInstance);
                if (roomInstance)
                {
                    invalidateRooms.insert(roomInstance);

                    RoomTileConstructor* roomConstructor = roomInstance->GetTileConstructor();
                    roomConstructor->ConstructRoomWalls(roomInstance, cxx::span{&currentTile, 1}, iface);
                }
            }
        }

        if (Room* roomInstance = currentTile->mRoomInstance)
        {
            invalidateRooms.insert(roomInstance);

            RoomTileConstructor* roomConstructor = roomInstance->GetTileConstructor();
            roomConstructor->ConstructRoomFloor(roomInstance, cxx::span{&currentTile, 1});
        }
    }

    // refresh heightmap last
    for (MapTile* currentTile: mInvalidatedTiles)
    {
        UpdateTileFloorHeightmap(currentTile);
        for (TileFaceData& tileface: currentTile->mFaces)
        {
            tileface.mFaceMeshDirty = false;
        }
    }
    // refresh navigation
    gNavigationService.UpdateAreaCodes(mInvalidatedTiles);
    mInvalidatedTiles.clear();
    // handle rooms objects arrangement
    for (Room* currentRoom: invalidateRooms)
    {
        currentRoom->RearrangeObjects();
    }
}

void GameWorld::ResetInvalidatedTiles()
{
    GameMap::TilesIterator tilesIterator = gGameMap.IterateTiles();
    for (MapTile* mapTile = tilesIterator.NextTile(); mapTile; 
        mapTile = tilesIterator.NextTile())
    {
        for (TileFaceData& tileFace: mapTile->mFaces)
        {
            tileFace.mFaceMeshDirty = false;
        }
    }
    mInvalidatedTiles.clear();
}

void GameWorld::InitTilesFloorHeightmap()
{
    GameMap::TilesIterator tilesIterator = gGameMap.IterateTiles();
    for (MapTile* mapTile = tilesIterator.NextTile(); mapTile; 
        mapTile = tilesIterator.NextTile())
    {
        UpdateTileFloorHeightmap(mapTile);
    }
}

void GameWorld::UpdateTileFloorHeightmap(MapTile* mapTile)
{
    const TileFaceMesh& floorMesh = mapTile->mFaces[eTileFace_Floor].mFaceMesh;
    TileHeightmap& floorHeightmap = mapTile->mFloorHeightmap;

    cxx::ray3d_t processRay;
    processRay.mDirection = -WorldAxes::Y;
    processRay.mOrigin = MapUtils::ComputeBlockCoordinate(mapTile->mLocation);
    processRay.mOrigin.y = MAP_FLOOR_LEVEL + MAP_BLOCK_HEIGHT + 1.0f;

    static float invResolution = 1.0f / (TileHeightmap::Resolution * 1.0f);

    float sampleStep = MAP_TILE_SIZE * invResolution;

    // sample center of subtile
    processRay.mOrigin.x += sampleStep * 0.5f;
    processRay.mOrigin.z += sampleStep * 0.5f;

    glm::vec3 outPoint;
    auto sampleHeight = [&floorMesh, &outPoint](const cxx::ray3d_t& sampleRay, float nonFloorThreshold) -> float
        {
            float height = 0.0f;
            for (const TileFaceMesh::Piece& currentPiece: floorMesh.mPieces)
            {
                const glm::ivec3* triangles = floorMesh.mTriangles.data() + currentPiece.mTrianglesOffset;
                const TerrainVertex3D* vertices = floorMesh.mVertices.data() + currentPiece.mBaseVertex;
                for (unsigned int itri = 0; itri < currentPiece.mTriangleCount; ++ itri)
                { 
                    const glm::ivec3& currentTriangle = triangles[itri];
                    bool found = cxx::intersects(sampleRay, 
                        vertices[currentTriangle[0]].mPosition,
                        vertices[currentTriangle[1]].mPosition,
                        vertices[currentTriangle[2]].mPosition, outPoint);

                    if (!found) continue;
                    if (outPoint[1] > nonFloorThreshold) continue;
                    if (outPoint[1] > height)
                    {
                        height = outPoint[1];
                    }
                }
            }
            return height;
        };
    const float nonFloorThreshold = MAP_FLOOR_LEVEL + (MAP_BLOCK_HEIGHT * 0.25f);
    const float originStartX = processRay.mOrigin.x;
    for (int iSampleY = 0; iSampleY < TileHeightmap::Resolution; ++iSampleY)
    {
        float* samplesPtr = floorHeightmap.mSamples + (iSampleY * TileHeightmap::Resolution);
        for (int iSampleX = 0; iSampleX < TileHeightmap::Resolution; ++iSampleX)
        {
            samplesPtr[iSampleX] = sampleHeight(processRay, nonFloorThreshold);
            processRay.mOrigin.x += sampleStep;
        }
        processRay.mOrigin.x = originStartX;
        processRay.mOrigin.z += sampleStep;
    }
}

void GameWorld::UpdateTileTaggedState(MapTile* mapTile, ePlayerID playerId, bool isTagged)
{
    cxx_assert(mapTile);
    cxx_assert(playerId > ePlayerID_Null);
    if (playerId == ePlayerID_Null)
        return;

    auto updateTileTaggedStateImpl = [mapTile](ePlayerID pid, bool isTagged)
        {
            if (mapTile->IsTaggedForDigging(pid) == isTagged)
                return;

            mapTile->SetTaggedForDigging(pid, isTagged);
            gCreatureTaskManager.OnTileTaggedStateChanged(mapTile, pid);
            if (pid == gGameSession.GetLocalPlayerId())
            {
                gGameRenderer.mTerrainRenderer.TileHighlightChanged(mapTile);
            }
        };

    // force for all players
    if (playerId == ePlayerID_COUNT)
    {
        for (int roller = ePlayerID_Null + 1; roller < ePlayerID_COUNT; ++roller)
        {
            const ePlayerID rollerId = static_cast<ePlayerID>(roller);
            updateTileTaggedStateImpl(rollerId, isTagged);
        }
    }
    else
    {
        updateTileTaggedStateImpl(playerId, isTagged);
    }
}

template<typename TContainer, typename TFilterFunc>
void GameWorld::ScanAdjacentRooms(cxx::span<MapTile*> tilesToScan, ePlayerID ownerId, TFilterFunc filterFunc, TContainer& container) const
{
    container.clear();

    for (MapTile* currentTile: tilesToScan)
    {
        for (eDirection direction: gStraightDirections)
        {
            MapTile* neighbourTile = currentTile->mNeighbours[direction];
            if (neighbourTile == nullptr) continue; // invalid tile
            if (neighbourTile->mOwnerId != ownerId) continue; // owner mismatch

            Room* roomInstance = neighbourTile->mRoomInstance;
            if (roomInstance == nullptr) continue; // no room there
            // check if already processed
            if (cxx::contains(container, neighbourTile->mRoomInstance)) 
                continue;

            if (filterFunc(roomInstance))
            {
                container.push_back(roomInstance);
            }
        } // for directions
    }
}

template<typename TEnumProc>
void GameWorld::EnumRoomSegments(Room* roomInstance, TEnumProc enumProc)
{
    cxx_assert(roomInstance);

    // todo: optimize
    Temp_Set<MapTile*> processedTiles;
    Temp_Vector<MapTile*> segmentTiles;
    Temp_Vector<MapTile*> coveredTiles = TempVectorFrom(roomInstance->GetFloorTiles());// intent copy
    for (MapTile* targetTile: coveredTiles)
    {
        if (processedTiles.find(targetTile) != processedTiles.end()) // already processed
            continue;

        segmentTiles.clear();
        gGameMap.FloodFill4(segmentTiles, targetTile);
        // add to processed tiles
        processedTiles.insert(segmentTiles.begin(), segmentTiles.end());
        enumProc(segmentTiles);
    }
}

bool GameWorld::CanTagTileForDigging(const MapPoint2D& mapLocation) const
{
    return gGameMap.WithinMap(mapLocation) && CanTagTileForDigging(gGameMap.GetMapTile(mapLocation));
}

bool GameWorld::CanTagTileForDigging(MapTile* mapTile) const
{
    if (mapTile)
    {
        TerrainDefinition* terrainDef = mapTile->GetTerrain();
        return terrainDef->mIsSolid && terrainDef->mIsTaggable;
    }
    return false;
}

int GameWorld::ChangeTileHealth(MapTile* mapTile, ePlayerID playerId, int hitpoints)
{
    cxx_assert(mapTile);

    if (mapTile == nullptr)
        return 0;

    TerrainDefinition* terrainDef = mapTile->GetTerrain();
    if (terrainDef->mIsImpenetrable)
        return 0;

    int deltaHitPoints = mapTile->ChangeHitPoints(hitpoints);
    if (deltaHitPoints == 0)
        return 0;

    // always invalidate
    InvalidateTile(mapTile);

    // handle terrain type changed
    TerrainTypeId changeTerrainType = TerrainTypeId_Null;
    if (mapTile->GetHitPoints() == 0)
    {
        changeTerrainType = terrainDef->mBecomesTerrainTypeWhenDestroyed;
    }
    if (mapTile->GetHitPoints() == mapTile->GetHitPointsMax())
    {
        changeTerrainType = terrainDef->mBecomesTerrainTypeWhenMaxHealth;
    }

    if ((changeTerrainType != TerrainTypeId_Null) && 
        (changeTerrainType != terrainDef->mTerrainType))
    {
        TerrainDefinition* newTerrain = gGameSession.GetScenarioDefinition().GetTerrainDefinition(changeTerrainType);
        cxx_assert(newTerrain);
        mapTile->SetBaseTerrain(newTerrain);
        // change owner
        // todo: notify owner about change
        mapTile->mOwnerId = newTerrain->mIsOwnable ? playerId : ePlayerID_Neutral;

        // force reset tagged state
        if (!CanTagTileForDigging(mapTile))
        {
            UpdateTileTaggedState(mapTile, ePlayerID_COUNT, false);
        }

        Temp_Set<Room*> rooms;
        for (eDirection direction: gStraightDirections)
        {
            MapTile* neighbourTile = mapTile->mNeighbours[direction];
            if (neighbourTile && neighbourTile->mRoomInstance)
            {
                rooms.insert(neighbourTile->mRoomInstance);
            }
        }
        for (Room* room: rooms)
        {
            room->NeighbourTileChanged(mapTile);
        }
        InvalidateTileNeighbours(mapTile);

        gCreatureTaskManager.OnTileTerrainTypeChanged(mapTile);
    }
    return deltaHitPoints;
}

bool GameWorld::QueryAccessibleMoneyStorageRoomsForDeposit(ePlayerID playerId, EntityHandle agentEntity, int maxRooms, 
    std::vector<EntityHandle>& outEntities)
{
    outEntities.clear();
    outEntities.reserve(16);

    // process creature
    if (agentEntity.IsCreature())
    {
        Player& player = gGameSession.GetPlayer(playerId);
        cxx_assert(!player.IsNonPlayer());

        Creature* creature = gCreatureManager.GetCreaturePtr(agentEntity);
        if ((creature == nullptr) || creature->WasDeleted())
            return false;

        const MapPoint2D startTileCoord = creature->GetTilePosition();
        const ePassabilityType passabilityType = creature->GetPassabilityType();

        for (const EntityHandle& roomHandle: player.GetOwnedMoneyStorageRooms())
        {
            Room* roomInstance = gRoomManager.GetRoomPtr(roomHandle);
            if ((roomInstance == nullptr) || roomInstance->WasDeleted())
            {
                continue;
            }

            // check if room is reachable
            cxx::span<MapTile*> roomTiles = roomInstance->GetFloorTiles();
            if (roomTiles.empty())
            {
                continue;
            }

            if (!gNavigationService.CheckPathExists(startTileCoord, roomTiles[0]->mLocation, passabilityType))
                continue;
           
            auto* moneyStorage = roomInstance->GetCapability<MoneyStorageRoomCapability>();
            cxx_assert(moneyStorage);
            if (moneyStorage == nullptr)
            {
                continue;
            }

            const long storageCapacity = moneyStorage->GetStoredGoldCapacity();
            bool isStorageFull = (storageCapacity > 0) && (moneyStorage->GetStoredGoldAmount() >= storageCapacity);
            if (isStorageFull)
            {
                continue;
            }
            outEntities.push_back(roomInstance->GetOwnHandle());
            
            // check limits
            if (maxRooms == static_cast<int>(outEntities.size()))
            {
                break;
            }
        }
    }
    else
    {
        cxx_assert(false);
    }
    return !outEntities.empty();
}


