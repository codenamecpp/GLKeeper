#pragma once

//////////////////////////////////////////////////////////////////////////

#include "RoomController.h"

//////////////////////////////////////////////////////////////////////////

class StorageRoomController: public RoomController
    , protected ObjectStorageRoomCapability
{
protected:

    //////////////////////////////////////////////////////////////////////////

    struct RoomStorageTile
    {
    public:
        static const int MaxObjectsPerTile = 3;

    public:
        Point2D mTileLocation {};

        // single storage tile can hold up to MaxObjectsPerTile objects 
        // depending on the room type the limit may be less than the MaxObjectsPerTile
        cxx::static_vector<EntityHandle, MaxObjectsPerTile> mObjects;
    };

    //////////////////////////////////////////////////////////////////////////

public:
    StorageRoomController(int maxObjectsPerStorageTile);

    // override RoomController
    void PostReconfigureRoom() override;

    // override ObjectStorageRoomCapability
    int GetStoredObjectsCount() override;
    int GetStoredObjectsCapacity() override;

    // pool
    void OnRecycle() override;

protected:
    inline int GetMaxObjectsPerStorageTile() const { return mMaxObjectsPerStorageTile; }
    inline int GetStorageTilesCount() const
    {
        return static_cast<int>(mStorageTiles.size());
    }
    int GetRoomStorageTileIndex(const Point2D& tileLocation) const;
    RoomStorageTile* GetRoomStorageTileFromLocation(const Point2D& tileLocation);
    RoomStorageTile* GetRoomStorageTileWithObject(EntityHandle entityHandle);
    RoomStorageTile* GetFirstAvailableStorageTile();

    inline const auto& GetStorageTiles() const { return mStorageTiles; }

    // assing/unassign stored objects manually
    // note: will fail if the object is already stored elsewhere
    // note: does not call StoredObjectAssigned / StoredObjectUnassigned
    bool AssignObjectToStorageTile(EntityHandle entityHandle, RoomStorageTile& storageTile);
    bool UnassignObjectFromStorage(EntityHandle entityHandle);

    // overridables
    virtual void EvaluateStorageTiles(cxx::any_vector<Point2D> evaluationResult) const = 0;
    virtual void StoredObjectUnassigned(EntityHandle entityHandle) = 0;
    virtual void StoredObjectReassigned(EntityHandle entityHandle, const RoomStorageTile& newStorageTile) = 0;
    virtual void StoredObjectAssigned(EntityHandle entityHandle, const RoomStorageTile& newStorageTile) = 0;

private:
    void SyncStorageTilesCacheAfterRoomReconfigure();
    void HandleRoomStorageTiles(cxx::span<Point2D> evaluatedTiles);

private:
    const int mMaxObjectsPerStorageTile;

    std::vector<RoomStorageTile> mStorageTiles; // cache
};

//////////////////////////////////////////////////////////////////////////