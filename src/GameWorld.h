#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameWorldDefs.h"
#include "SceneObject.h"
#include "MapSelectionCursor.h"
#include "TileConstructor.h"
#include "TileConstructionSet.h"
#include "GameObjectDefs.h"
#include "CreatureDefs.h"
#include "RoomDefs.h"
#include "PhysicsDefs.h"
#include "NavigationDefs.h"

//////////////////////////////////////////////////////////////////////////

class GameWorld final: public cxx::noncopyable
{
public:
    // Load session data, level map, setup players, build rooms etc
    bool LoadScenario(const ScenarioDefinition& scenarioDefinition, GameLoadingAware& loadingContext);
    // on start / end of each game session
    void EnterWorld();
    void ClearWorld();

    // update world
    void UpdateFrame(float deltaTime);
    void UpdateLogic(float stepDeltaTime);
    void UpdatePhysics(float stepDeltaTime);

    // accessing world related managers
    inline TileConstructionSet& GetTileConstructionSet() { return mTileConstructionSet; }
    inline MapSelectionCursor& GetMapSelectionCursor() { return mMapSelectionCursor; }

    // get frame statistics
    const WorldStatistics& GetStatistics() const { return mPrevFrameStats; }

    // demolish room(s) within map area
    // adjacent rooms may be split, shrunk or destructed
    bool DemolishRooms(ePlayerID playerId, const Rect2D& mapArea, cxx::any_vector<MapTile*> demolishTiles);
    bool CanDemolishRooms(ePlayerID playerId, const Rect2D& mapArea, cxx::any_vector<MapTile*> demolishTiles) const;

    // constructs a room(s) within map area
    // adjacent rooms may be merged, split, or absorbed
    bool ConstructRooms(ePlayerID playerId, RoomDefinition* roomDefinition, const Rect2D& mapArea, 
        cxx::any_vector<MapTile*> constructionTiles);
    bool CanConstructRooms(ePlayerID playerId, RoomDefinition* roomDefinition, const Rect2D& mapArea, 
        cxx::any_vector<MapTile*> constructionTiles) const;

    // check whether tiles tile can be tagged for digging for player
    bool CanTagTileForDigging(const Point2D& mapLocation) const;
    bool CanTagTileForDigging(MapTile* mapTile) const;

    // mark tiles as tagged for digging for player
    void TagTilesForDigging(const Rect2D& tileArea, ePlayerID playerId, bool setTagged);

    // Process mining on a solid block if it is a mineable vein (gold, gems)
    // Out param 'goldMined' contains the amount of resource mined
    // Returns false if the operation cannot be performed
    bool MineBlock(MapTile* mapTile, ePlayerID playerId, long& goldMined, float changeHealthMultiplier = 1.0f);
    bool CanMineBlock(MapTile* mapTile, ePlayerID playerId) const;

    // Process digging on a non-impenetrable solid block (rock, reinforced wall etc)
    // Does nothing if it's mineable vein
    // Returns false if the operation cannot be performed
    bool DigBlock(MapTile* mapTile, ePlayerID playerId, float changeHealthMultiplier = 1.0f);
    bool CanDigBlock(MapTile* mapTile, ePlayerID playerId) const;

    // Process wall reinforcement on a solid block (rock)
    // Does nothing if it is already already reinforced wall
    // Returns false if the operation cannot be performed
    bool ReinforceWall(MapTile* mapTile, ePlayerID playerId);
    bool CanReinforceWall(MapTile* mapTile, ePlayerID playerId) const;

    // Process floor tile claiming (dirt path, mana vault)
    // Does nothing if is already claimed
    // Returns false if the operation cannot be performed
    bool ClaimTile(MapTile* mapTile, ePlayerID playerId);
    bool CanClaimTile(MapTile* mapTile, ePlayerID playerId) const;

    // Process attack damage on attackable tile (reinforced wall or claimed path, including room tile)
    // Does nothing if it's not a room tile or room is not attackable
    // Returns false if the operation cannot be performed
    bool DamageTile(MapTile* mapTile, ePlayerID playerId, float changeHealthMultiplier = 1.0f);
    bool CanDamageTile(MapTile* mapTile, ePlayerID playerId) const;

    //////////////////////////////////////////////////////////////////////////

    // When tile geometry gets modified it must then be synchronized with physics and rendering systems, its need to be manually invalidated
    // Geometry of invalidated tiles will be Clear and then Reconstructed
    // Rooms themselves must handle invalidated tiles, Reconstruct it geometry, but without Clear
    void InvalidateTile(MapTile* mapTile);
    void InvalidateTile(MapTile* mapTile, eTileFace face);
    // Full invalidation (all faces) of tiles in array
    void InvalidateTiles(cxx::span<MapTile*> mapTiles);
    // Invalidate tiles surrounding target tile in all directions
    // If neighbour tile is solid, single wall side will be invalidated but if solid tile is in diagonal direction it will be skipped
    void InvalidateTileNeighbours(MapTile* mapTile);

    //////////////////////////////////////////////////////////////////////////

    // generate new entity unique id
    EntityUid GenerateEntityUid();

private:
    template<typename TContainer, typename TFilterFunc>
    void ScanAdjacentRooms(cxx::span<MapTile*> tilesToScan, ePlayerID ownerId, TFilterFunc filterFunc, TContainer& container) const;

    template<typename TEnumProc>
    void EnumRoomSegments(Room* roomInstance, TEnumProc enumProc);

    // Explore room area and create room instance from it
    void CreateRoomFromUnexploredTiles(MapTile* startTile, const ScenarioDefinition& scenarioDefinition);

    void CreateEnvironmentEntities();

    bool CheckBordersWithOwnedTerritory(MapTile* mapTile, ePlayerID playerId) const;

    // Tiles will be no more part of specified room instance
    // Will create additional rooms for separated parts of original room
    void ReleaseRoomTiles(Room* roomInstance, cxx::span<MapTile*> roomTiles);

    void HandleRoomAbsorbed(Room* roomInstance);
    void HandleRoomCollapsed(Room* roomInstance);
    void HandleRoomReclaimed(Room* roomInstance, ePlayerID playerId);

    // returns delta hitpoints applied
    bool ChangeTileHealth(MapTile* mapTile, ePlayerID playerId, int hitpoints, int& healthDelta);

    void BuildInvalidatedTiles();
    void ResetInvalidatedTiles();

    // force recompute heightmap of all map tiles
    void InitTilesFloorHeightmap();
    void UpdateTileFloorHeightmap(MapTile* mapTile);

    void UpdateTileTaggedState(MapTile* mapTile, ePlayerID playerId, bool isTagged);

private:
    EntityUid mNextEntityUid = 1;

    MapSelectionCursor mMapSelectionCursor;
    TileConstructionSet mTileConstructionSet;

    std::vector<MapTile*> mInvalidatedTiles;

    WorldStatistics mCurrentFrameStats;
    WorldStatistics mPrevFrameStats;

    std::vector<cxx::uniqueptr<EnvironmentMeshObject>> mEnvironmentObjects;
};

//////////////////////////////////////////////////////////////////////////

extern GameWorld gGameWorld;

//////////////////////////////////////////////////////////////////////////