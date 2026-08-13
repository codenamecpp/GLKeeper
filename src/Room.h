#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameDefs.h"
#include "RoomDefs.h"
#include "ScenarioDefs.h"
#include "RoomWallSection.h"
#include "Entity.h"
#include "RoomComponents.h"
#include "RoomCapabilities.h"

//////////////////////////////////////////////////////////////////////////

class TileConstructor;
class RoomTileConstructor;

//////////////////////////////////////////////////////////////////////////
    
// Generic Room

class Room final 
    : public Entity
    , public EnableEntityComponents<RoomComponents>
    , public EnableEntityCapabilities<RoomCapabilities>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // lifecycle
    //////////////////////////////////////////////////////////////////////////

    // performs early initialization of the instance, before it is spawned
    // called by RoomManager during CreateRoom()
    void ConfigureInstance(EntityUid instanceUid, 
        RoomController* controller, RoomTileConstructor* tileConstructor, 
        RoomDefinition* definition, eDirection direction, ePlayerID owner);

    // preloads resources, adds the instance to the game world and activates it
    // called by RoomManager during ActivateRoom()
    void SpawnInstance();

    // deactivates the instance and releases its internal resources
    // called by RoomManager during DeleteRoom()
    void DespawnInstance();

    // think
    void UpdateLogic(float stepDeltaTime);

    // mark room as pending deletion
    void MarkDeleted();

    // pool
    void OnRecycle();

    //////////////////////////////////////////////////////////////////////////
    // control
    //////////////////////////////////////////////////////////////////////////

    // Move tiles and objects from other room to this room, other room becomes empty
    // @param sourceRoom: Donor
    // @param targetTiles: Target tiles to absorb
    void AbsorbRoom(Room* sourceRoom, cxx::span<MapTile*> targetTiles);
    void AbsorbRoom(Room* sourceRoom);

    // Remove specified room tiles, this will lead to reevaluation of remaining tiles
    void ReleaseTiles(cxx::span<MapTile*> targetTiles);
    void ReleaseTiles();

    // Add specified uncovered tiles to room, this will lead to reevaluation of remaining tiles
    void EnlargeRoom(cxx::span<MapTile*> targetTiles);

    // This is rather complex function
    // It will destroy unneeded objects, create new objects and correct positions inside room
    // Make sure to refresh geometries and terrain heightfield first
    void RearrangeObjects();

    // Handle situation when adjacent solid tils is reinforced or destroyed, so room must reevaluate
    // its walls and add or demove objects
    void NeighbourTileChanged(MapTile* targetTile);

    // accessing room tile constructor
    inline RoomTileConstructor* GetTileConstructor() const { return mTileConstructor; }

    // get definition
    inline RoomDefinition* GetDefinition() const { return mDefinition; }

    // definition shortcuts
    inline RoomTypeId GetTypeId() const { return mDefinition->mRoomType; }

    // accessing room wall sections for reading
    inline cxx::span<const RoomWallSection*> GetWallSections() const { return mWallSections; }

    // accessing room floor tiles
    inline cxx::span<MapTile*> GetFloorTiles() const { return mCoveredTiles; }
    inline cxx::span<MapTile*> GetInnerTiles() const { return mInnerTiles; }

    inline cxx::span<const RoomFurnitureSlot> GetFloorFurniture() const { return mFloorFurniture; }
    inline cxx::span<const RoomFurnitureSlot> GetWallsFurniture() const { return mWallsFurniture; }
    inline cxx::span<const RoomFurnitureSlot> GetPillars() const { return mPillars; }

    inline cxx::span<const RoomStorageSlot> GetStorageSlots() const { return mStorageSlots; }

    // approximate room size in tiles
    const MapArea2D& GetLocationArea() const { return mLocationArea; }

    inline ePlayerID GetOwnerId() const { return mOwnerId; }

    inline eDirection GetRoomDirection() const { return mRoomDirection; }

    // get room overall size in tiles
    inline int GetRoomSize() const 
    { 
        return static_cast<int>(mCoveredTiles.size()); 
    }

    // get room inner tiles count
    inline int GetRoomInnerSize() const 
    { 
        return static_cast<int>(mInnerTiles.size()); 
    }

private:
    // test whether room can be constructed on specific tile
    bool CanConstructOn(const MapTile* targetTile);

    // append or remove tiles to room instance
    void AssignTiles(cxx::span<MapTile*> targetTiles);
    void UnassignTiles(cxx::span<MapTile*> targetTiles);
    void ReevaluateOccupationArea();
    void ReevaluateInnerSquares();
    void ReevaluateWallSections();

    bool TransferRoomObjectsTo(Room* receiver, cxx::span<MapTile*> targetTiles);
    bool TransferRoomObjectsTo(Room* receiver);

    // actualize room own objects (furniture/pillars): adds new or removes old
    void HandleRoomFurnitureObjects(cxx::span<RoomFurnitureSlot> newObjects, RoomFurnitureSlots& prevObjects);
    void PostRearrangeObjects();
    void DeleteFurnitureObjects();

    // each time room gets modified it must be reconfigurated
    // for example, tiles added or removed, wall sections updated
    void Reconfigure();

    void ScanWallSection(MapTile* mapTile, eTileFace faceId, RoomWallSection* section) const;
    void ScanWallSection(MapTile* mapTile, eDirection faceDirection, RoomWallSection* section) const;
    void ScanWallSectionImpl(MapTile* mapTile, RoomWallSection* section) const;
    void ReleaseWallSections();
    void DetachFromWall(MapTile* roomTile);
    void FinalizeWallSection(RoomWallSection* section);
    RoomWallSection* FindWallSectionWithTile(MapTile* mapTile, eTileFace face) const;

    // storage slots
    friend class StorageRoomController;
    void AssignObjectToStorageSlot(const MapPoint2D& tileLocation, EntityHandle entityHandle);
    void UnassignStorageSlotObject(const MapPoint2D& tileLocation, EntityHandle entityHandle);
    void ReassignStorageSlotObject(const MapPoint2D& tileLocation, EntityHandle entityHandle, const MapPoint2D& newLocation);
    int GetStorageSlotIndex(EntityHandle entityHandle) const;

private:   
    ePlayerID mOwnerId = ePlayerID_Null;

    eDirection mRoomDirection = eDirection_N;

    RoomDefinition* mDefinition = nullptr;
    RoomTileConstructor* mTileConstructor = nullptr;
    RoomController* mController = nullptr;

    MapArea2D mLocationArea; // approximate room size in tiles
    std::vector<MapTile*> mInnerTiles;
    std::vector<MapTile*> mCoveredTiles;
    std::vector<RoomWallSection*> mWallSections;

    // furniture objects
    RoomFurnitureSlots mFloorFurniture;
    RoomFurnitureSlots mWallsFurniture;
    RoomFurnitureSlots mPillars;
    RoomFurnitureSlots mTempFurnitureEvaluation;

    // list of currently stored objects
    std::vector<RoomStorageSlot> mStorageSlots;
};

//////////////////////////////////////////////////////////////////////////