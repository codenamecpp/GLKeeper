#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameWorldDefs.h"
#include "SceneObject.h"
#include "TileSelectionOutline.h"
#include "GameMap.h"
#include "TileConstructor.h"
#include "Scene.h"
#include "TileConstructionSet.h"
#include "GameObjectDefs.h"
#include "CreatureDefs.h"
#include "RoomDefs.h"
#include "PhysicsDefs.h"
#include "NavigationDefs.h"
#include "GameSessionAware.h"

//////////////////////////////////////////////////////////////////////////

class GameWorld final: private GameSessionAware
{
public:
    GameWorld();
    ~GameWorld();

    // Load session data, level map, setup players, build rooms etc
    bool LoadScenario(const ScenarioDefinition& scenarioDefinition, GameLoadingAware& loadingContext);
    // on start / end of each game session
    void EnterWorld();
    void ClearWorld();

    // update world
    void UpdateFrame(float deltaTime);
    void UpdateLogic(float stepDeltaTime);
    void UpdatePhysics(float stepDeltaTime);

    // accessing scene
    inline Scene& GetScene() { return mScene; }

    // accessing primary camera state
    inline Camera& GetMainCamera() { return GetScene().GetCamera(); }

    // accessing map info and tile data
    inline GameMap& GetGameMap() { return mGameMap; }

    // accessing dungeon builder
    inline TileConstructionSet& GetTileConstructionSet() { return mTileConstructionSet; }

    // accessing tile selection outline
    inline TileSelectionOutline& GetTileSelectionOutline() { return mTileSelectionOutline; }

    // accessing current world statistics
    const WorldStatistics& GetStatistics() const { return mPrevFrameStats; }

    // accessing game objects
    inline GameObjectManager& GetGameObjects() { return *mGameObjects; }

    // accessing room manager
    inline RoomManager& GetRoomManager() { return *mRoomManager; }

    // accessing creature manager
    inline CreatureManager& GetCreatureManager() { return *mCreatureManager; }

    // accessing navigation service
    inline NavigationService& GetNavigationService() { return *mNavigationService; }

    // accessing world physics
    inline Physics& GetPhysics() { return *mPhysics; }

    // Test whether room is buildable on specific spot
    bool CanPlaceRoomOnLocation(MapTile* mapTile, ePlayerID playerID, RoomDefinition* roomDefinition) const;

    // Test whether room is sellable on specific spot
    bool CanSellRoomOnLocation(MapTile* mapTile, ePlayerID playerID) const;

    // check whether terrain tile can be tagged
    bool CanTagTerrain(const MapPoint2D& mapLocation) const;
    bool CanTagTerrain(MapTile* mapTile) const;

    // mark tile as tagged
    void TagTerrain(const MapArea2D& tileArea);

    // clear tile tagged state
    void UnTagTerrain(const MapArea2D& tileArea);

    // Sell rooms and objects within specified area, will split rooms
    void SellEntities(ePlayerID playerID, const MapArea2D& tilesArea);

    // Build room within specified area, will merge contiguous rooms
    void ConstructRoom(ePlayerID playerID, RoomDefinition* roomDefinition, const MapArea2D& tilesArea);

    // Do damage tile, it will also damage room at this location
    void DamageTile(MapTile* mapTile, ePlayerID playerID, int hitPoints);

    // Repair or claim tile, it will also fix walls of surrounding rooms
    void RepairTile(MapTile* mapTile, ePlayerID playerID, int hitPoints);

    // Cast ray in specific viewport coordinate using current camera
    bool CastRayFromScreenPoint(const Point2D& screenCoordinate, cxx::ray3d_t& resultRay);

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

    void BuildInvalidatedTiles();
    void ResetInvalidatedTiles();

    // force recompute heightmap of all map tiles
    void InitTilesFloorHeightmap();
    void UpdateTileFloorHeightmap(MapTile* mapTile);

    void SetTileTagged(MapTile* mapTile, bool isTagged);

private:
    Scene mScene;

    EntityUid mNextEntityUid = 1;

    TileSelectionOutline mTileSelectionOutline;
    TileConstructionSet mTileConstructionSet;

    std::vector<MapTile*> mInvalidatedTiles;

    GameMap mGameMap;
    std::unique_ptr<GameObjectManager> mGameObjects;
    std::unique_ptr<RoomManager> mRoomManager;
    std::unique_ptr<CreatureManager> mCreatureManager;
    std::unique_ptr<Physics> mPhysics;
    std::unique_ptr<NavigationService> mNavigationService;

    WorldStatistics mCurrentFrameStats;
    WorldStatistics mPrevFrameStats;

    std::vector<cxx::uniqueptr<EnvironmentMeshObject>> mEnvironmentObjects;
};

//////////////////////////////////////////////////////////////////////////