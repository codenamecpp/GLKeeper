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

//////////////////////////////////////////////////////////////////////////

GameWorld gGameWorld;

//////////////////////////////////////////////////////////////////////////

GameWorld::GameWorld()
{
    // allocate services
    mCreatureManager    = std::make_unique<CreatureManager>();
    mRoomManager        = std::make_unique<RoomManager>();
    mGameObjects        = std::make_unique<GameObjectManager>();
    mPhysics            = std::make_unique<Physics>();
    mNavigationService  = std::make_unique<NavigationService>();
}

GameWorld::~GameWorld()
{

}

bool GameWorld::LoadScenario(const ScenarioDefinition& scenarioDefinition, GameLoadingAware& loadingContext)
{
    GetScene().Initialize();

    loadingContext.UpdateLoadingProgress(0.0f);

    // configure game time
    gTime.ResetFixedClock(eFixedClock::GameLogic);
    gTime.SetFixedClockFramerate(eFixedClock::GameLogic, scenarioDefinition.mTicksPerSecond);

    mTileConstructionSet.Initialize(scenarioDefinition);

    loadingContext.UpdateLoadingProgress(0.1f);

    GetPhysics().LoadScenario(scenarioDefinition);
    GetRoomManager().LoadScenario(scenarioDefinition);

    loadingContext.UpdateLoadingProgress(0.15f);

    GetGameMap().LoadScenario(scenarioDefinition);

    loadingContext.UpdateLoadingProgress(0.2f);

    { // create rooms
        GameMap::TilesIterator tilesIterator = GetGameMap().IterateTiles();
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

        GameMap::TilesIterator tilesIterator = GetGameMap().IterateTiles();
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
    GetRoomManager().ProcessRoomChanges();
    for (Room* roomInstance: GetRoomManager().GetRooms())
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

    GetGameObjects().LoadScenario(scenarioDefinition);

    loadingContext.UpdateLoadingProgress(0.9f);

    GetCreatureManager().LoadScenario(scenarioDefinition);

    mTileSelectionOutline.Init(GetScene());

    GetMainCamera().ResetOrientation();

    EnterWorld();

    loadingContext.UpdateLoadingProgress(1.0f);
    return true;
}

void GameWorld::EnterWorld()
{
    GetPhysics().EnterWorld();
    GetNavigationService().EnterWorld();
    GetRoomManager().EnterWorld();
    GetGameObjects().EnterWorld();
    GetCreatureManager().EnterWorld();

    mCurrentFrameStats.Clear();
    mPrevFrameStats.Clear();
}

void GameWorld::ClearWorld()
{   
    mEnvironmentObjects.clear();
    mTileSelectionOutline.Deinit();
    GetRoomManager().ClearWorld();
    GetCreatureManager().ClearWorld();
    GetGameObjects().ClearWorld();
    GetNavigationService().ClearWorld();
    GetPhysics().ClearWorld();
    gGameRenderer.mTerrainRenderer.CleanupTerrainMesh();
    GetGameMap().Cleanup();
    GetScene().ClearScene();
    mTileConstructionSet.Cleanup();
    mNextEntityUid = 1;

    GetScene().Shutdown();
}

void GameWorld::UpdateFrame(float deltaTime)
{
    mPrevFrameStats = mCurrentFrameStats;

    mCurrentFrameStats.Clear();
    mCurrentFrameStats.mNumSceneObjectsActive = mScene.GetActiveSceneObjectCount();

    mTileSelectionOutline.UpdateFrame();

    BuildInvalidatedTiles();

    // tick those subsystems after refresh terrain geometries
    GetGameObjects().UpdateFrame(deltaTime);
    GetCreatureManager().UpdateFrame(deltaTime);
    GetRoomManager().UpdateFrame(deltaTime);
    GetNavigationService().UpdateFrame(deltaTime);
    GetPhysics().UpdateFrame(deltaTime);
    GetScene().UpdateFrame(deltaTime);
}

void GameWorld::UpdateLogic(float stepDeltaTime)
{
    GetGameObjects().UpdateLogic(stepDeltaTime);
    GetRoomManager().UpdateLogic(stepDeltaTime);
    GetCreatureManager().UpdateLogic(stepDeltaTime);
    GetNavigationService().UpdateLogic(stepDeltaTime);
}

void GameWorld::UpdatePhysics(float stepDeltaTime)
{
    GetCreatureManager().UpdatePhysics(stepDeltaTime);
    GetGameObjects().UpdatePhysics(stepDeltaTime);
    GetPhysics().UpdatePhysics(stepDeltaTime);
}

bool GameWorld::CastRayFromScreenPoint(const Point2D& screenCoordinate, cxx::ray3d_t& resultRay)
{
    const Viewport& viewport = gRenderDevice.GetViewport();

    Camera& sceneCamera = GetMainCamera();
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
    GameMap::TilesIterator tilesIterator = GetGameMap().IterateTiles();
    for (MapTile* currMapTile = tilesIterator.NextTile(); currMapTile; currMapTile = tilesIterator.NextTile())
    {
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

    // create water and lava surfaces
    while (!lavaTiles.empty())
    {
        MapTile* originTile = lavaTiles.back();
        lavaTiles.pop_back();

        tempTiles.clear();
        GetGameMap().FloodFill4(tempTiles, originTile, FLOOD_FILL4_SAME_BASE_TERRAIN);
        if (tempTiles.empty())
        {
            cxx_assert(false);
            continue;
        }

        cxx::uniqueptr<EnvironmentMeshObject> lavaSurface = GetScene().CreateLavaMesh(tempTiles);
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
        GetGameMap().FloodFill4(tempTiles, originTile, FLOOD_FILL4_SAME_BASE_TERRAIN);
        if (tempTiles.empty())
        {
            cxx_assert(false);
            continue;
        }

        cxx::uniqueptr<EnvironmentMeshObject> waterSurface = GetScene().CreateWaterMesh(tempTiles);
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
    RoomDefinition* roomDefinition = GetScenarioDefinition().GetRoomDefinitionByTerrain(startTile->GetTerrain());
    cxx_assert(roomDefinition);
    if (!roomDefinition)
        return;

    Temp_Vector<MapTile*> floodTiles;
    GetGameMap().FloodFill4(floodTiles, startTile);

    // get room extents
    MapPoint2D minPoint = startTile->mTileLocation;
    MapPoint2D maxPoint = startTile->mTileLocation;
    for (MapTile* roller: floodTiles)
    {
        minPoint = glm::min(minPoint, roller->mTileLocation);
        maxPoint = glm::max(maxPoint, roller->mTileLocation);
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

    const ePlayerID ownerID = roomInitParams ? roomInitParams->mPlayerId : startTile->mOwnerID;

    // create room instance
    EntityHandle roomHandle = roomInitParams ?
        GetRoomManager().CreateScenarioRoom(*roomInitParams) :
        GetRoomManager().CreateRoom(roomDefinition, ownerID);

    GetGameSession().GetPlayer(ownerID).AddToInventory(roomHandle);
    GetRoomManager().ActivateRoom(roomHandle);
    if (Room* roomInstance = GetRoomManager().GetRoomPtr(roomHandle))
    {
        roomInstance->EnlargeRoom(floodTiles);
    }
}

void GameWorld::DamageTile(MapTile* mapTile, ePlayerID playerID, int hitPoints)
{
    cxx_assert(mapTile);
    cxx_assert(hitPoints > 0);

    if (mapTile->mRoomInstance)
    {
        ReleaseRoomTiles(mapTile->mRoomInstance, {&mapTile, 1});
    }

    TerrainDefinition* terrain = mapTile->GetTerrain();
    if (terrain->mIsLava || terrain->mIsWater || terrain->mIsImpenetrable)
        return;

    // todo: tile hp

    TerrainTypeId newTerrainType = terrain->mBecomesTerrainTypeWhenDestroyed;
    if (terrain->mTerrainType == newTerrainType)
        return;

    TerrainDefinition* newTerrain = GetScenarioDefinition().GetTerrainDefinition(newTerrainType);
    mapTile->mBaseTerrain = newTerrain;

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

    // force update tagged state
    if (!CanTagTerrain(mapTile))
    {
        SetTileTagged(mapTile, false);
    }

    InvalidateTile(mapTile);
    InvalidateTileNeighbours(mapTile);
}

void GameWorld::RepairTile(MapTile* mapTile, ePlayerID playerID, int hitPoints)
{
    cxx_assert(mapTile);
    cxx_assert(hitPoints > 0);

    TerrainDefinition* terrain = mapTile->GetTerrain();
    if (terrain->mIsLava || terrain->mIsWater || terrain->mIsImpenetrable)
        return;

    // todo: tile hp

    if (mapTile->mRoomInstance)
        return;

    TerrainTypeId newTerrainType = terrain->mBecomesTerrainTypeWhenMaxHealth;
    if (terrain->mTerrainType == newTerrainType)
        return;

    TerrainDefinition* newTerrain = GetScenarioDefinition().GetTerrainDefinition(newTerrainType);
    mapTile->mBaseTerrain = newTerrain;
    mapTile->mOwnerID = playerID;
    
    Temp_Set<Room*> rooms;
    for (eDirection direction: gStraightDirections)
    {
        MapTile* neighbourTile = mapTile->mNeighbours[direction];
        if (neighbourTile && neighbourTile->mRoomInstance)
        {
            rooms.insert(neighbourTile->mRoomInstance);
        }
    }

    for (Room* genericRoom: rooms)
    {
        genericRoom->NeighbourTileChanged(mapTile);
    }

    InvalidateTile(mapTile);
    InvalidateTileNeighbours(mapTile);
}

void GameWorld::TagTerrain(const MapArea2D& tileArea)
{
    auto tilesIterator = GetGameMap().IterateTiles(tileArea);
    for (MapTile* mapTile = tilesIterator.NextTile(); mapTile; 
        mapTile = tilesIterator.NextTile())
    {
        if (CanTagTerrain(mapTile))
        {
            SetTileTagged(mapTile, true);
        }
    }
}

void GameWorld::UnTagTerrain(const MapArea2D& tileArea)
{
    auto tilesIterator = GetGameMap().IterateTiles(tileArea);
    for (MapTile* mapTile = tilesIterator.NextTile(); mapTile; 
        mapTile = tilesIterator.NextTile())
    {
        if (mapTile->mIsTagged)
        {
            SetTileTagged(mapTile, false);
        }
    }
}

bool GameWorld::CanPlaceRoomOnLocation(MapTile* mapTile, ePlayerID playerID, RoomDefinition* roomDefinition) const
{
    cxx_assert(mapTile);

    TerrainDefinition* tileTerrain = mapTile->GetTerrain();
    if (GetScenarioDefinition().IsRoomTypeTerrain(tileTerrain))
        return false;

    // cannot place on mana vault
    if (tileTerrain->mTerrainType == TerrainTypeId_ClaimedVault) 
        return false;

    if (roomDefinition->mPlaceableOnLand)
    {
        if (!tileTerrain->mIsSolid && tileTerrain->mIsOwnable && (mapTile->mOwnerID == playerID))
            return true;
    }

    if (roomDefinition->mPlaceableOnLava && tileTerrain->mIsLava)
        return true;

    if (roomDefinition->mPlaceableOnWater && tileTerrain->mIsWater)
        return true;

    return false;
}

bool GameWorld::CanSellRoomOnLocation(MapTile* mapTile, ePlayerID playerID) const
{
    cxx_assert(mapTile);

    Room* roomInstance = mapTile->mRoomInstance;
    if (!roomInstance || playerID != mapTile->mOwnerID)
        return false;

    RoomDefinition* roomDefinition = roomInstance->GetDefinition();
    return (roomDefinition->mBuildable == true);
}

void GameWorld::SellEntities(ePlayerID playerID, const MapArea2D& tilesArea)
{
    Temp_Vector<MapTile*> roomTiles;
    roomTiles.reserve(64);

    auto tilesIterator = GetGameMap().IterateTiles(tilesArea);
    for (MapTile* mapTile = tilesIterator.NextTile(); mapTile; 
        mapTile = tilesIterator.NextTile())
    {
        if (CanSellRoomOnLocation(mapTile, playerID))
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

void GameWorld::ConstructRoom(ePlayerID playerID, RoomDefinition* roomDefinition, const MapArea2D& tilesArea)
{
    // collect all tiles available to construction, output array is sorted

    Temp_Vector<MapTile*> constructionTiles;
    constructionTiles.reserve(tilesArea.w * tilesArea.h);

    auto tilesIterator = GetGameMap().IterateTiles(tilesArea);
    for (MapTile* mapTile = tilesIterator.NextTile(); mapTile; 
        mapTile = tilesIterator.NextTile())
    {
        if (CanPlaceRoomOnLocation(mapTile, playerID, roomDefinition))
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
        GetGameMap().FloodFill4(segmentTiles, currentTile, tilesArea);
        // put into processed set
        for (MapTile* processedTile: segmentTiles)
        {
            processedTiles.insert(processedTile);
        }

        // find rooms that contacting with current segment and merge them all

        adjacentRooms.clear();
        ScanAdjacentRooms(segmentTiles, playerID, 
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
            EntityHandle roomHandle = GetRoomManager().CreateRoom(roomDefinition, playerID);
            GetGameSession().GetPlayer(playerID).AddToInventory(roomHandle);
            GetRoomManager().ActivateRoom(roomHandle);
            absorberRoom = GetRoomManager().GetRoomPtr(roomHandle);
        }

        // add segment tiles to room
        for (MapTile* processedTile: segmentTiles)
        {
            cxx_assert(processedTile->mRoomInstance == nullptr);
            processedTile->mTerrain = GetScenarioDefinition().GetTerrainDefinition(roomDefinition->mTerrainType);
            processedTile->mOwnerID = playerID;
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
        roomTile->mTerrain = nullptr;
        if (!roomDefinition->mPlaceableOnLand && 
            (roomDefinition->mPlaceableOnLava || roomDefinition->mPlaceableOnWater))
        {   
            roomTile->mOwnerID = ePlayerID_Neutral; // for bridges terrain reset owner
        }
    }

    int segmentsCounter = 0;
    EnumRoomSegments(currentRoom, [this, &segmentsCounter, currentRoom](cxx::span<MapTile*> segmentTiles)
    {
        if (segmentsCounter++ == 0) // first segment is current room segment
            return;

        ePlayerID ownerID = currentRoom->GetOwnerID();

        // each new segment is a new room
        EntityHandle newRoom = GetRoomManager().CreateRoom(currentRoom->GetDefinition(), ownerID);
        GetGameSession().GetPlayer(ownerID).AddToInventory(newRoom);
        GetRoomManager().ActivateRoom(newRoom);
        if (Room* roomInstance = GetRoomManager().GetRoomPtr(newRoom))
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
    ePlayerID ownerID = roomInstance->GetOwnerID();
    GetGameSession().GetPlayer(ownerID).RemoveFromInventory(roomInstance->GetOwnHandle());
    GetRoomManager().DeleteRoom(roomInstance->GetInstanceUid());
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
    ePlayerID ownerID = roomInstance->GetOwnerID();
    GetGameSession().GetPlayer(ownerID).RemoveFromInventory(roomInstance->GetOwnHandle());
    GetRoomManager().DeleteRoom(roomInstance->GetInstanceUid());
}

void GameWorld::BuildInvalidatedTiles()
{
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
    mInvalidatedTiles.clear();

    // handle rooms objects arrangement
    for (Room* currentRoom: invalidateRooms)
    {
        currentRoom->RearrangeObjects();
    }
}

void GameWorld::ResetInvalidatedTiles()
{
    GameMap::TilesIterator tilesIterator = GetGameMap().IterateTiles();
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
    GameMap::TilesIterator tilesIterator = GetGameMap().IterateTiles();
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
    processRay.mOrigin = MapUtils::ComputeBlockCoordinate(mapTile->mTileLocation);
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

void GameWorld::SetTileTagged(MapTile* mapTile, bool isTagged)
{
    cxx_assert(mapTile);
    if (mapTile && (mapTile->mIsTagged != isTagged))
    {
        mapTile->mIsTagged = isTagged;
        gGameRenderer.mTerrainRenderer.TileHighlightChanged(mapTile); 
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
            if (neighbourTile->mOwnerID != ownerId) continue; // owner mismatch

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
    Temp_Vector<MapTile*> coveredTiles = MakeTempVector(roomInstance->GetFloorTiles());// intent copy
    for (MapTile* targetTile: coveredTiles)
    {
        if (processedTiles.find(targetTile) != processedTiles.end()) // already processed
            continue;

        segmentTiles.clear();
        GetGameMap().FloodFill4(segmentTiles, targetTile);
        // add to processed tiles
        processedTiles.insert(segmentTiles.begin(), segmentTiles.end());
        enumProc(segmentTiles);
    }
}

bool GameWorld::CanTagTerrain(const MapPoint2D& mapLocation) const
{
    return mGameMap.WithinMap(mapLocation) && CanTagTerrain(mGameMap.GetMapTile(mapLocation));
}

bool GameWorld::CanTagTerrain(MapTile* mapTile) const
{
    if (mapTile)
    {
        TerrainDefinition* terrainDef = mapTile->GetTerrain();
        return terrainDef->mIsTaggable;
    }
    return false;
}
