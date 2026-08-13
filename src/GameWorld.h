#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameWorldDefs.h"
#include "SceneObject.h"
#include "TileSelectionOutline.h"
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
    inline TileSelectionOutline& GetTileSelectionOutline() { return mTileSelectionOutline; }

    // get frame statistics
    const WorldStatistics& GetStatistics() const { return mPrevFrameStats; }

    // Test whether room is buildable on specific spot
    bool CanPlaceRoomOnLocation(MapTile* mapTile, ePlayerID playerId, RoomDefinition* roomDefinition) const;

    // Test whether room is sellable on specific spot
    bool CanSellRoomOnLocation(MapTile* mapTile, ePlayerID playerId) const;

    // check whether tiles tile can be tagged for digging for player
    bool CanTagTileForDigging(const MapPoint2D& mapLocation) const;
    bool CanTagTileForDigging(MapTile* mapTile) const;

    // mark tiles as tagged for digging for player
    void TagTilesForDigging(const MapArea2D& tileArea, ePlayerID playerId, bool setTagged);

    // Sell rooms and objects within specified area, will split rooms
    void SellEntities(ePlayerID playerId, const MapArea2D& tilesArea);

    // Build room within specified area, will merge contiguous rooms
    void ConstructRoom(ePlayerID playerId, RoomDefinition* roomDefinition, const MapArea2D& tilesArea);

    // Process digging operation on a solid block
    // outGoldMined contains the amount of gold mined, if any
    // Returns false if the operation cannot be performed on the tile
    bool DigTile(MapTile* mapTile, ePlayerID playerId, long& outGoldMined);

    // Process wall reinforcement on a solid block
    // outCompleted will be set to true if wall reinforcement is finished
    // Returns false if the operation cannot be performed on the tile
    bool ReinforceWall(MapTile* mapTile, ePlayerID playerId, bool& outCompleted);

    // Process claim floor on tile
    // outCompleted will be set to true if floor claimed
    // Returns false if the operation cannot be performed on the tile
    bool ClaimFloor(MapTile* mapTile, ePlayerID playerId, bool& outCompleted);

    // Do damage/repair tile
    void DamageTile(MapTile* mapTile, ePlayerID playerId, int hitPoints);
    void RepairTile(MapTile* mapTile, ePlayerID playerId, int hitPoints);

    // Cast ray in specific viewport coordinate using current camera
    bool CastRayFromScreenPoint(const Point2D& screenCoordinate, cxx::ray3d_t& resultRay);

    //////////////////////////////////////////////////////////////////////////

    // Queries

    template<typename TEntitiesList>
    bool QueryAccessibleMoneyStorageRoomsForDeposit(ePlayerID playerId, EntityHandle agentEntity, int maxRooms, TEntitiesList& outEntities);
    bool QueryAccessibleMoneyStorageRoomsForDeposit(ePlayerID playerId, EntityHandle agentEntity, int maxRooms, std::vector<EntityHandle>& outEntities);

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

    // Tiles will be no more part of specified room instance
    // Will create additional rooms for separated parts of original room
    void ReleaseRoomTiles(Room* roomInstance, cxx::span<MapTile*> roomTiles);

    void HandleRoomAbsorbed(Room* roomInstance);
    void HandleRoomCollapsed(Room* roomInstance);

    // returns delta hitpoints applied
    int ChangeTileHealth(MapTile* mapTile, ePlayerID playerId, int hitpoints);

    void BuildInvalidatedTiles();
    void ResetInvalidatedTiles();

    // force recompute heightmap of all map tiles
    void InitTilesFloorHeightmap();
    void UpdateTileFloorHeightmap(MapTile* mapTile);

    void UpdateTileTaggedState(MapTile* mapTile, ePlayerID playerId, bool isTagged);

private:
    EntityUid mNextEntityUid = 1;

    TileSelectionOutline mTileSelectionOutline;
    TileConstructionSet mTileConstructionSet;

    std::vector<MapTile*> mInvalidatedTiles;

    WorldStatistics mCurrentFrameStats;
    WorldStatistics mPrevFrameStats;

    std::vector<EntityHandle> mTempQueryEntities;

    std::vector<cxx::uniqueptr<EnvironmentMeshObject>> mEnvironmentObjects;
};

//////////////////////////////////////////////////////////////////////////

extern GameWorld gGameWorld;

//////////////////////////////////////////////////////////////////////////

template<typename TEntitiesList>
bool GameWorld::QueryAccessibleMoneyStorageRoomsForDeposit(ePlayerID playerId, EntityHandle agentEntity, int maxRooms, TEntitiesList& outEntities)
{
    bool isSuccess = QueryAccessibleMoneyStorageRoomsForDeposit(playerId, agentEntity, maxRooms, mTempQueryEntities);
    if (isSuccess)
    {
        outEntities.assign(std::begin(mTempQueryEntities), std::end(mTempQueryEntities));
        mTempQueryEntities.clear();
    }
    return isSuccess;
}

//////////////////////////////////////////////////////////////////////////