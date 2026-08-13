#pragma once

//////////////////////////////////////////////////////////////////////////

#include "CreatureDefs.h"
#include "ScenarioDefs.h"

//////////////////////////////////////////////////////////////////////////

class CreatureTaskManager final: public cxx::noncopyable
{
public:
    CreatureTaskManager();
    ~CreatureTaskManager();

    bool LoadScenario(const ScenarioDefinition& scenarioDef);
    void EnterWorld();
    void ClearWorld();

    void UpdateFrame(float deltaTime);
    void UpdateLogic(float stepDeltaTime);

    //////////////////////////////////////////////////////////////////////////

    // create wander task and add creature to task's assignees list
    //
    // fails if there are no passable tiles around the creature
    CreatureTaskPtr GetWanderTask(Creature* assignee);

    // worker tasks

    CreatureTaskPtr GetDiggingTask(Creature* assignee);
    CreatureTaskPtr GetMiningTask(Creature* assignee);
    CreatureTaskPtr GetClaimFloorTask(Creature* assignee);
    CreatureTaskPtr GetReinforceWallTask(Creature* assignee);
    CreatureTaskPtr GetCarryGoldToTreasuryTask(Creature* assignee);

    //////////////////////////////////////////////////////////////////////////

public:

    //////////////////////////////////////////////////////////////////////////

    // processing events

    void OnTileTaggedStateChanged(MapTile* mapTile, ePlayerID playerId);
    void OnTileTerrainTypeChanged(MapTile* mapTile);

    //////////////////////////////////////////////////////////////////////////

private:
    // factory
    CreatureTaskPtr NewTaskInstance() const;
    CreatureTaskUid GenerateTaskUid();

    CreatureTaskPtr GetWorkerTask(Creature* assignee, eCreatureJob jobType);

    void UnregisterTask(CreatureTask* creatureTask);

    bool IsWorkerJob(eCreatureJob jobType) const;

    void ProcessTileTerrainTypeChanges();

    void ProcessTileTaggedStateChanges();
    void UpdateTaggedTileTasks(MapTile* mapTile, ePlayerID playerId);
    void RemoveTaggedTileTasks(MapTile* mapTile, ePlayerID playerId);

    void InitReinforceWallTasks(MapTile* mapTile, ePlayerID playerId);
    void UpdateReinforceWallTasks(MapTile* mapTile, ePlayerID playerId);
    bool CanReinforceWall(MapTile* mapTile, ePlayerID playerId, TileFaceIdSet& faceSet) const;

    void InitClaimFloorTasks(MapTile* mapTile, ePlayerID playerId);
    void UpdateClaimFloorTasks(MapTile* mapTile, ePlayerID playerId);
    bool CanClaimFloor(MapTile* mapTile, ePlayerID playerId) const;

    void CreateWorkerSlotsForJob(MapTile* mapTile, eCreatureJob jobType, eTileFace tileFace, ePlayerID playerId);
    void RemoveWorkerSlotsForJob(MapTile* mapTile, eCreatureJob jobType, eTileFace tileFace, ePlayerID playerId);
    void RemoveWorkerSlotsForJob(MapTile* mapTile, eCreatureJob jobType, ePlayerID playerId);
        
private:
    CreatureTaskUid mNextTaskUid = 1;

    std::vector<std::pair<ePlayerID, MapTile*>> mTaggedTileStateChanges;
    std::vector<MapTile*> mTileTerrainTypeChanges;

    // some tasks allow up to 3 workers per tile face so each one gets its own slot
    struct WorkerSlot
    {
        glm::vec2 mPosition {};
        CreatureTask* mCreatureTask {}; // non null if assigned
    };

    struct TileWithWorkerSlots
    {
        MapTile* mMapTile {}; // T
        eCreatureJob mJobType {};
        /*
                |v v v|
            ____|_____|____
              > |     | <
              > |  T  | <
            __>_|_____|_<__
                |     |
                |^ ^ ^|
        */
        std::vector<WorkerSlot> mWorkerSlots[eTileFace_COUNT]; // v > < ^
    };

    struct WorkerTasks
    {
        std::vector<TileWithWorkerSlots> mTiles;
    };
    WorkerTasks mPlayersWorkerTasks[ePlayerID_COUNT];   
};

//////////////////////////////////////////////////////////////////////////

extern CreatureTaskManager gCreatureTaskManager;

//////////////////////////////////////////////////////////////////////////
