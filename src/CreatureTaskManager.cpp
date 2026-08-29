#include "stdafx.h"
#include "CreatureTaskManager.h"
#include "SimplePool.h"
#include "CreatureTask.h"
#include "NavigationService.h"
#include "Creature.h"
#include "MapTile.h"
#include "MapUtils.h"
#include "GameWorld.h"
#include "RoomManager.h"
#include "GameSession.h"
#include "GameMap.h"

//////////////////////////////////////////////////////////////////////////

CreatureTaskManager gCreatureTaskManager;

//////////////////////////////////////////////////////////////////////////

inline bool IsKeeperPlayerId(ePlayerID playerId)
{
    return (playerId < ePlayerID_COUNT) && (playerId >= ePlayerID_Keeper1);
}

//////////////////////////////////////////////////////////////////////////

CreatureTaskManager::CreatureTaskManager()
{
}

CreatureTaskManager::~CreatureTaskManager()
{
}

CreatureTaskPtr CreatureTaskManager::NewTaskInstance() const
{
    static SimplePool<CreatureTask> pool = (
        [](CreatureTask* instance)
        {
            instance->OnRecycle();
        });

    CreatureTask* instance = pool.Acquire();
    return std::move(CreatureTaskPtr (instance, [](CreatureTask* creatureTask)
        {
            if (creatureTask)
            {
                gCreatureTaskManager.UnregisterTask(creatureTask);
                pool.Return(creatureTask);
            }
        }));
}

bool CreatureTaskManager::LoadScenario(const ScenarioDefinition& scenarioDef)
{
    return true;
}

void CreatureTaskManager::EnterWorld()
{
    // init tasks

    for (Player& playersRoller: gGameSession.GetPlayers())
    {
        if (!playersRoller.IsKeeperPlayer())
            continue;

        GameMap::TilesIterator tilesIterator = gGameMap.IterateTiles();
        for (MapTile* mapTile = tilesIterator.NextTile(); mapTile; 
            mapTile = tilesIterator.NextTile())
        {
            InitReinforceWallTasks(mapTile, playersRoller.GetPlayerId());
            InitClaimTerritoryTasks(mapTile, playersRoller.GetPlayerId());
        }
    }
}

void CreatureTaskManager::ClearWorld()
{
    mNextTaskUid = 1;
    for (WorkerTasks& roller: mPlayersWorkerTasks)
    {
        roller.mTiles.clear();
    }
    mTaggedTileStateChanges.clear();
    mTileTerrainTypeChanges.clear();
}

bool CreatureTaskManager::IsWorkerJob(eCreatureJob jobType) const
{
    switch (jobType)
    {
        case eCreatureJob_Dig:
        case eCreatureJob_Mine:
        case eCreatureJob_Claim:
        case eCreatureJob_ReinforceWall:
        case eCreatureJob_RepairWall:
        case eCreatureJob_CarryCorpseToGraveyard:
        case eCreatureJob_CarryEnemyToPrison:
        case eCreatureJob_CarryFallenToLair:
        case eCreatureJob_CarrySpecialToLibrary:
        case eCreatureJob_CarryGoldToTreasury:
        case eCreatureJob_CarryCrateToWorkshop:
        case eCreatureJob_InstallTrap:
        case eCreatureJob_InstallDoor:
            return true;
    }
    return false;
}

void CreatureTaskManager::UpdateFrame(float deltaTime)
{
    ProcessChanges();
}

void CreatureTaskManager::UpdateLogic(float stepDeltaTime)
{
}

CreatureTaskPtr CreatureTaskManager::GetWanderTask(Creature* assignee)
{
    cxx_assert(assignee);
    if (assignee == nullptr)
        return nullptr;

    // todo: figure out correct values
    const int wanderMinDistance = 3;
    const int wanderMaxDistance = 5;

    glm::vec2 wanderPosition;
    if (!gNavigationService.GetRandomWanderingPoint(assignee->GetPosition2d(), assignee->GetPassabilityType(), 
        wanderMinDistance, 
        wanderMaxDistance, wanderPosition))
    {
        // todo: report
        return nullptr;
    }

    // allocate task
    CreatureTaskPtr creatureTask = NewTaskInstance();
    creatureTask->Configure(eCreatureJob_Wander, GenerateTaskUid(), assignee);
    creatureTask->ConfigureTargetPosition(wanderPosition);
    return std::move(creatureTask);
}

CreatureTaskPtr CreatureTaskManager::GetDiggingTask(Creature* assignee)
{
    return GetWorkerTask(assignee, eCreatureJob_Dig);
}

CreatureTaskPtr CreatureTaskManager::GetMiningTask(Creature* assignee)
{
    return GetWorkerTask(assignee, eCreatureJob_Mine);
}

CreatureTaskPtr CreatureTaskManager::GetReinforceWallTask(Creature* assignee)
{
    return GetWorkerTask(assignee, eCreatureJob_ReinforceWall);
}

CreatureTaskPtr CreatureTaskManager::GetClaimTerritoryTask(Creature* assignee)
{
    return GetWorkerTask(assignee, eCreatureJob_Claim);
}

CreatureTaskUid CreatureTaskManager::GenerateTaskUid()
{
    const CreatureTaskUid resultUid = mNextTaskUid++;
    return resultUid;
}

void CreatureTaskManager::OnTileTaggedStateChanged(MapTile* mapTile, ePlayerID playerId)
{
    cxx_assert(mapTile);
    cxx_assert(playerId < ePlayerID_COUNT);
    if (mapTile == nullptr)
        return;

    if (cxx::contains_if(mTaggedTileStateChanges, [playerId, mapTile](const auto& roller)
        {
            return (roller.first == playerId) && (roller.second == mapTile);
        }))
    {
        return;
    }
    mTaggedTileStateChanges.emplace_back(playerId, mapTile);
}

void CreatureTaskManager::ProcessTileTaggedStateChanges()
{
    if (mTaggedTileStateChanges.empty())
        return;

    for (const auto& roller: mTaggedTileStateChanges)
    {
        UpdateTaggedTileTasks(roller.second, roller.first);

        // if the block was claimed for digging, discard reinforce wall tasks for it
        UpdateReinforceWallTasks(roller.second, roller.first);
    }
    mTaggedTileStateChanges.clear();
}

void CreatureTaskManager::UpdateTaggedTileTasks(MapTile* mapTile, ePlayerID playerId)
{
    cxx_assert(mapTile);
    cxx_assert(playerId < ePlayerID_COUNT);

    if (!mapTile->IsTaggedForDigging(playerId))
    {
        // force remove related tasks
        RemoveTaggedTileTasks(mapTile, playerId);
        return;
    }

    const TerrainDefinition* terrainDefs = mapTile->GetTerrain();
    if (!terrainDefs->mIsSolid || !terrainDefs->mIsTaggable)
    {
        // force remove related tasks
        RemoveTaggedTileTasks(mapTile, playerId);
        return;
    }

    const eCreatureJob jobType = (terrainDefs->mGoldValue > 0) ? eCreatureJob_Mine : eCreatureJob_Dig;

    TileFaceIdSet availableFacesSet;

    static const eTileFace TileFaces[] = { eTileFace_SideN, eTileFace_SideE, eTileFace_SideS, eTileFace_SideW };
    for (eTileFace facesRoller: TileFaces)
    {
        eDirection dir = TileFaceToDirection(facesRoller);
        MapTile* neighbourTile = mapTile->mNeighbours[dir];
        if (neighbourTile == nullptr)
            continue;

        const TerrainDefinition* neighbourDefs = neighbourTile->GetTerrain();
        if (neighbourDefs->mIsSolid)
            continue;

        if (neighbourDefs->mIsLava)
        {
            // todo: what if we have custom workers that can walk on lava?
            continue;
        }

        // todo: check for solid objects on tile

        availableFacesSet.Include(facesRoller);
    }

    if (availableFacesSet.Empty())
    {
        RemoveWorkerSlotsForJob(mapTile, jobType, playerId);
        return;
    }

    for (eTileFace facesRoller: TileFaces)
    {
        if (availableFacesSet.Contains(facesRoller))
        {
            CreateWorkerSlotsForJob(mapTile, jobType, facesRoller, playerId);
        }
        else
        {
            RemoveWorkerSlotsForJob(mapTile, jobType, facesRoller, playerId);
        }
    }
}

void CreatureTaskManager::RemoveTaggedTileTasks(MapTile* mapTile, ePlayerID playerId)
{
    cxx_assert(mapTile);

    RemoveWorkerSlotsForJob(mapTile, eCreatureJob_Mine, playerId);
    RemoveWorkerSlotsForJob(mapTile, eCreatureJob_Dig, playerId);
}

void CreatureTaskManager::CreateWorkerSlotsForJob(MapTile* mapTile, eCreatureJob jobType, eTileFace tileFace, ePlayerID playerId)
{
    cxx_assert(mapTile);
    cxx_assert((tileFace < eTileFace_COUNT) && (tileFace != eTileFace_Ceiling));
    cxx_assert(IsKeeperPlayerId(playerId));
    cxx_assert((jobType == eCreatureJob_Claim) ? (tileFace == eTileFace_Floor) : (tileFace != eTileFace_Floor));
    
    std::vector<TileWithWorkerSlots>& tilesList = mPlayersWorkerTasks[playerId].mTiles;
    
    int tileListIndex = cxx::get_first_index_if(tilesList, 
        [mapTile, jobType](const auto& roller)
        {
            return (roller.mMapTile == mapTile) && (roller.mJobType == jobType);
        });
    if (tileListIndex == -1)
    {
        tileListIndex = static_cast<int>(tilesList.size());
        TileWithWorkerSlots& tileWithSlots = tilesList.emplace_back();
        tileWithSlots.mJobType = jobType;
        tileWithSlots.mMapTile = mapTile;
    }

    unsigned int maxWorkersPerBlockSide = 1;
    switch (jobType)
    {
        case eCreatureJob_Dig:
        {
            const TerrainDefinition* terrainDef = mapTile->GetTerrain();
            maxWorkersPerBlockSide = terrainDef->mIsOwnable ? 3 : 1; // reinforced wall vs rock
        }
        break;
        case eCreatureJob_Mine:
            maxWorkersPerBlockSide = 3;
        break;
        case eCreatureJob_Claim:
        case eCreatureJob_ReinforceWall:
        case eCreatureJob_RepairWall:
            maxWorkersPerBlockSide = 1;
        break;
        default:
            cxx_assert(false);
        break;
    }

    std::vector<WorkerSlot>& workerSlotsList = tilesList[tileListIndex].mWorkerSlots[tileFace];
    if (workerSlotsList.size() > maxWorkersPerBlockSide)
    {
        // discard slots, cancel tasks
        while (workerSlotsList.size() < maxWorkersPerBlockSide)
        {
            if (CreatureTask* creatureTask = workerSlotsList.back().mCreatureTask)
            {
                creatureTask->SetExpired();
            }
            workerSlotsList.pop_back();
        }
    }
    else if (workerSlotsList.size() < maxWorkersPerBlockSide)
    {
        cxx::static_vector<glm::vec2, 3> availablePositions;
        if (tileFace == eTileFace_Floor)
        {
            availablePositions.push_back(MapUtils::ComputeTileCenter2d(mapTile->mLocation));
        }
        else // sides
        {
            glm::vec2 tileEdges[4];
            MapUtils::ComputeTileEdges2d(mapTile->mLocation, tileEdges);

            glm::vec2 p0;
            glm::vec2 p1;

            switch (tileFace)
            {
                case eTileFace_SideN: p0 = tileEdges[0]; p1 = tileEdges[1]; break;
                case eTileFace_SideE: p0 = tileEdges[1]; p1 = tileEdges[2]; break;
                case eTileFace_SideS: p0 = tileEdges[2]; p1 = tileEdges[3]; break;
                case eTileFace_SideW: p0 = tileEdges[0]; p1 = tileEdges[3]; break;
                default: 
                    cxx_assert(false);
                break;
            }

            const eDirection directionFromTileFace = TileFaceToDirection(tileFace);
            const glm::vec2 positionOffset = glm::vec2(gDirectionVectors[directionFromTileFace]) * MAP_TILE_HALF_SIZE * 0.5f;
            const glm::vec2 pcenter = (p0 + p1) * 0.5f;
            availablePositions.push_back(pcenter + positionOffset);
            availablePositions.push_back((pcenter + p0) * 0.5f + positionOffset);
            availablePositions.push_back((pcenter + p1) * 0.5f + positionOffset);
        }
        // add slots
        while (workerSlotsList.size() < maxWorkersPerBlockSide)
        {
            const unsigned int positionIndex = workerSlotsList.size() % availablePositions.size();
            WorkerSlot& addedSlot = workerSlotsList.emplace_back();
            addedSlot.mPosition = availablePositions[positionIndex];
            addedSlot.mCreatureTask = nullptr;
        }
    }
    else {} // same slots count
}

void CreatureTaskManager::RemoveWorkerSlotsForJob(MapTile* mapTile, eCreatureJob jobType, eTileFace tileFace, ePlayerID playerId)
{
    cxx_assert(mapTile);
    cxx_assert(IsKeeperPlayerId(playerId));

    std::vector<TileWithWorkerSlots>& tilesList = mPlayersWorkerTasks[playerId].mTiles;
    
    int tileListIndex = cxx::get_first_index_if(tilesList, 
        [mapTile, jobType](const auto& roller)
        {
            return (roller.mMapTile == mapTile) && (roller.mJobType == jobType);
        });

    if (tileListIndex == -1)
        return;

    std::vector<WorkerSlot>& workerSlotsList = tilesList[tileListIndex].mWorkerSlots[tileFace];
    // discard slots, cancel tasks
    for (WorkerSlot& slotsRoller: workerSlotsList)
    {
        if (slotsRoller.mCreatureTask)
        {
            slotsRoller.mCreatureTask->SetExpired();
        }
    }
    workerSlotsList.clear();
}

void CreatureTaskManager::RemoveWorkerSlotsForJob(MapTile* mapTile, eCreatureJob jobType, ePlayerID playerId)
{
    cxx_assert(mapTile);
    cxx_assert(IsKeeperPlayerId(playerId));

    std::vector<TileWithWorkerSlots>& tilesList = mPlayersWorkerTasks[playerId].mTiles;

    // discard slots, cancel tasks
    for (TileWithWorkerSlots& tilesRoller: tilesList)
    {
        if ((tilesRoller.mMapTile != mapTile) || 
            (tilesRoller.mJobType != jobType))
        {
            continue;
        }   
        
        for (eTileFace facesRoller : gTileFaces)
        {
            std::vector<WorkerSlot>& slotsList = tilesRoller.mWorkerSlots[facesRoller];
            for (WorkerSlot& slotsRoller: slotsList)
            {
                if (slotsRoller.mCreatureTask)
                {
                    slotsRoller.mCreatureTask->SetExpired();
                }
            }
            slotsList.clear();
        }
    }

    // remove tiles
    cxx::erase_if(tilesList, [mapTile, jobType](const TileWithWorkerSlots& tilesRoller)
        {
            return (tilesRoller.mMapTile == mapTile) && (tilesRoller.mJobType == jobType);
        });
}

void CreatureTaskManager::UnregisterTask(CreatureTask* creatureTask)
{
    cxx_assert(creatureTask);
    if (creatureTask == nullptr)
        return;

    creatureTask->SetExpired();

    if (IsWorkerJob(creatureTask->GetJobType()))
    {
        const ePlayerID playerId = creatureTask->GetPlayerId();
        cxx_assert(playerId < ePlayerID_COUNT);

        WorkerTasks& workerTasks = mPlayersWorkerTasks[playerId];
        for (TileWithWorkerSlots& tilesRoller: workerTasks.mTiles)
        {
            if (tilesRoller.mJobType != creatureTask->GetJobType())
                continue;

            for (eTileFace facesRoller : gTileFaces)
            {
                std::vector<WorkerSlot>& slotsList = tilesRoller.mWorkerSlots[facesRoller];
                for (WorkerSlot& slotsRoller: slotsList)
                {
                    if (slotsRoller.mCreatureTask != creatureTask)
                        continue;

                    // set unassigned
                    slotsRoller.mCreatureTask = nullptr;
                }
            }
        }

    } // if worker
}

void CreatureTaskManager::OnTileTerrainTypeChanged(MapTile* mapTile)
{
    cxx_assert(mapTile);
    if (mapTile == nullptr)
        return;

    if (!cxx::contains(mTileTerrainTypeChanges, mapTile))
    {
        mTileTerrainTypeChanges.push_back(mapTile);
    }
}

void CreatureTaskManager::ProcessTileTerrainTypeChanges()
{
    if (mTileTerrainTypeChanges.empty())
        return;

    for (MapTile* tilesRoller: mTileTerrainTypeChanges)
    {
        for (const Player& playersRoller: gGameSession.GetPlayers())
        {
            if (!playersRoller.IsKeeperPlayer())
                continue;

            const ePlayerID playerId = playersRoller.GetPlayerId();
            UpdateTaggedTileTasks(tilesRoller, playerId);
            UpdateReinforceWallTasks(tilesRoller, playerId);
            UpdateClaimTerritoryTasks(tilesRoller, playerId);

            // process neighbour tiles
            for (eDirection dirsRoller: gStraightDirections)
            {
                MapTile* neighbourTile = tilesRoller->mNeighbours[dirsRoller];
                if (neighbourTile == nullptr)
                    continue;

                UpdateTaggedTileTasks(neighbourTile, playerId);
                UpdateReinforceWallTasks(neighbourTile, playerId);
                UpdateClaimTerritoryTasks(neighbourTile, playerId);
            }
        }
    }
    mTileTerrainTypeChanges.clear();
}

CreatureTaskPtr CreatureTaskManager::GetWorkerTask(Creature* assignee, eCreatureJob jobType)
{
    cxx_assert(assignee);
    if (assignee == nullptr)
        return nullptr;

    // force process changes
    ProcessChanges();

    cxx_assert(IsWorkerJob(jobType));

    const ePlayerID playerId = assignee->GetOwnerId();
    const ePassabilityType passabilityType = assignee->GetPassabilityType();
    const glm::vec2 creaturePosition = assignee->GetPosition2d();
    const Point2D creatureTile = assignee->GetTilePosition();

    WorkerSlot* bestCandidateSlot = nullptr;
    MapTile* bestCandidateTargetTile = nullptr;
    float bestCandidateDistance2 = 0.0f;

    bool excludeInfiniteMoneySources = false;
    if ((jobType == eCreatureJob_Mine) && !assignee->CanCarryMoreMoney())
    {
        // if a worker requests a mining task while already unable to carry more,
        // it means there is currently nowhere to store the mined gold

        // gold veins should still be mined because they may be blocking access to other areas
        // but gems are an infinite source of gold, so they should not be mined in this case
        excludeInfiniteMoneySources = true;
    }

    std::vector<TileWithWorkerSlots>& tilesList = mPlayersWorkerTasks[playerId].mTiles;
    for (TileWithWorkerSlots& tilesRoller: tilesList)
    {
        if (tilesRoller.mJobType != jobType)
            continue;

        if (excludeInfiniteMoneySources && tilesRoller.mMapTile->IsImpenetrable())
            continue;

        for (eTileFace facesRoller: gTileFaces)
        {
            std::vector<WorkerSlot>& slotsList = tilesRoller.mWorkerSlots[facesRoller];
            if (slotsList.empty())
                continue;

            const float candidatesDistance2 = glm::distance2(slotsList.front().mPosition, creaturePosition);
            if (bestCandidateSlot && (candidatesDistance2 > bestCandidateDistance2))
                continue;

            for (WorkerSlot& slotsRoller: slotsList)
            {
                // skip if already assigned
                if (slotsRoller.mCreatureTask)
                    continue;

                // skip if not accessible
                const Point2D slotTile = MapUtils::ComputeTileFromPosition(slotsRoller.mPosition);
                if (!gNavigationService.CheckPathExists(creatureTile, slotTile, passabilityType))
                    continue;

                // pick candidate
                bestCandidateDistance2 = candidatesDistance2;
                bestCandidateTargetTile = tilesRoller.mMapTile;
                bestCandidateSlot = &slotsRoller;
                break;
            }
        }
    }

    CreatureTaskPtr creatureTask;
    if (bestCandidateSlot)
    {
        cxx_assert(bestCandidateTargetTile);

        creatureTask = NewTaskInstance();
        creatureTask->Configure(jobType, GenerateTaskUid(), assignee);
        creatureTask->ConfigureTargetPosition(bestCandidateSlot->mPosition);
        creatureTask->ConfigureTargetTile(bestCandidateTargetTile->mLocation);
        // bind to slot
        cxx_assert(bestCandidateSlot->mCreatureTask == nullptr);
        bestCandidateSlot->mCreatureTask = creatureTask.get();
    }
    return std::move(creatureTask);
}

CreatureTaskPtr CreatureTaskManager::GetCarryGoldToTreasuryTask(Creature* assignee)
{
    cxx_assert(assignee);
    if (assignee == nullptr)
        return nullptr;

    cxx::temp_vector<EntityHandle> roomEntities;
    if (!gGameWorld.QueryAccessibleMoneyStorageRoomsForDeposit(assignee->GetOwnerId(), assignee->GetOwnHandle(), 0, roomEntities))
        return nullptr;

    const Point2D creatureTile = assignee->GetTilePosition();

    std::optional<Point2D> closestStorageTile {};
    long long closestStorageDistance2 = 0;

    // find tile to store gold
    for (EntityHandle entitiesRoller: roomEntities)
    {
        Room* roomInstance = gRoomManager.GetRoomPtr(entitiesRoller);
        cxx_assert(roomInstance);
        if (roomInstance == nullptr)
            continue;

        if (roomInstance->GetOwnerId() != assignee->GetOwnerId())
        {
            cxx_assert(false);
            continue;
        }
        
        auto* storage = roomInstance->GetCapability<MoneyStorageRoomCapability>();
        cxx_assert(storage);
        if (storage == nullptr)
            continue;

        Point2D storageTile;
        if (!storage->GetTileToStoreGold(storageTile))
            continue;

        const long long distanceToStorage2 = cxx::distance2i(storageTile, creatureTile);
        if (!closestStorageTile || (distanceToStorage2 < closestStorageDistance2))
        {
            closestStorageTile.emplace(storageTile);
            closestStorageDistance2 = distanceToStorage2;
        }
    }

    CreatureTaskPtr creatureTask;
    if (closestStorageTile)
    {
        creatureTask = NewTaskInstance();
        creatureTask->Configure(eCreatureJob_CarryGoldToTreasury, GenerateTaskUid(), assignee);
        creatureTask->ConfigureTargetTile(*closestStorageTile);
    }
    return std::move(creatureTask);
}

void CreatureTaskManager::UpdateReinforceWallTasks(MapTile* mapTile, ePlayerID playerId)
{
    TileFaceIdSet faceSet;
    if (!CanReinforceWall(mapTile, playerId, faceSet))
    {
        RemoveWorkerSlotsForJob(mapTile, eCreatureJob_ReinforceWall, playerId);
        return;
    }

    for (eTileFace facesRoller: gTileFaces)
    {
        if (faceSet.Contains(facesRoller))
        {
            CreateWorkerSlotsForJob(mapTile, eCreatureJob_ReinforceWall, facesRoller, playerId);
        }
        else
        {
            RemoveWorkerSlotsForJob(mapTile, eCreatureJob_ReinforceWall, facesRoller, playerId);
        }        
    }
}

void CreatureTaskManager::InitReinforceWallTasks(MapTile* mapTile, ePlayerID playerId)
{
    TileFaceIdSet faceSet;
    if (!CanReinforceWall(mapTile, playerId, faceSet))
        return;

    for (eTileFace facesRoller: gTileFaces)
    {
        if (!faceSet.Contains(facesRoller))
            continue;

        CreateWorkerSlotsForJob(mapTile, eCreatureJob_ReinforceWall, facesRoller, playerId);
    }
}

bool CreatureTaskManager::CanReinforceWall(MapTile* mapTile, ePlayerID playerId, TileFaceIdSet& faceSet) const
{
    cxx_assert(mapTile);

    faceSet.Clear();

    if (!IsKeeperPlayerId(playerId))
        return false;

    // skip tile if it already at max health
    if (mapTile->GetHitPoints() >= mapTile->GetHitPointsMax())
        return false;

    // don't reinfore if it tagged for digging
    if (mapTile->IsTaggedForDigging(playerId))
        return false;

    // check terrain props

    const TerrainDefinition* terrainDef = mapTile->GetTerrain();
    if (!terrainDef->mIsSolid || terrainDef->mIsOwnable)
        return false;

    const TerrainTypeId nextTerrainTypeId = terrainDef->mBecomesTerrainTypeWhenMaxHealth;
    cxx_assert (nextTerrainTypeId != TerrainTypeId_Null);

    const TerrainDefinition* nextTerrainDef = gGameSession.GetScenarioDefinition().GetTerrainDefinition(nextTerrainTypeId);
    cxx_assert(nextTerrainDef);
    if (!nextTerrainDef->mIsSolid || !nextTerrainDef->mIsOwnable)
        return false;

    // check faces
    for (eTileFace facesRoller: {eTileFace_SideN, eTileFace_SideE, eTileFace_SideS, eTileFace_SideW})
    {
        const eDirection dir = TileFaceToDirection(facesRoller);

        MapTile* neighbourTile = mapTile->mNeighbours[dir];
        if (neighbourTile == nullptr)
            continue;

        const TerrainDefinition* neighbourTileDef = neighbourTile->GetTerrain();
        if (neighbourTileDef->mIsSolid || !neighbourTileDef->mIsOwnable)
            continue;

        if (neighbourTile->HasOwner(playerId))
        {
            faceSet.Include(facesRoller);
        }
    }

    return !faceSet.Empty();
}

bool CreatureTaskManager::CanClaimTerritory(MapTile* mapTile, ePlayerID playerId) const
{
    cxx_assert(mapTile);
    return IsKeeperPlayerId(playerId) && gGameWorld.CanClaimTile(mapTile, playerId);
}

void CreatureTaskManager::InitClaimTerritoryTasks(MapTile* mapTile, ePlayerID playerId)
{
    if (CanClaimTerritory(mapTile, playerId))
    {
        CreateWorkerSlotsForJob(mapTile, eCreatureJob_Claim, eTileFace_Floor, playerId);
    }
}

void CreatureTaskManager::UpdateClaimTerritoryTasks(MapTile* mapTile, ePlayerID playerId)
{
    if (!CanClaimTerritory(mapTile, playerId))
    {
        RemoveWorkerSlotsForJob(mapTile, eCreatureJob_Claim, playerId);
    }
    else
    {
        CreateWorkerSlotsForJob(mapTile, eCreatureJob_Claim, eTileFace_Floor, playerId);
    }
}

void CreatureTaskManager::ProcessChanges()
{
    ProcessTileTaggedStateChanges();
    ProcessTileTerrainTypeChanges();
}
