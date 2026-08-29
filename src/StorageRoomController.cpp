#include "stdafx.h"
#include "StorageRoomController.h"
#include "GameObjectManager.h"

StorageRoomController::StorageRoomController(int maxObjectsPerStorageTile)
    : mMaxObjectsPerStorageTile(maxObjectsPerStorageTile)
{
    cxx_assert(mMaxObjectsPerStorageTile <= RoomStorageTile::MaxObjectsPerTile);
    cxx_assert(mMaxObjectsPerStorageTile > 0);
}

void StorageRoomController::PostReconfigureRoom()
{
    RoomController::PostReconfigureRoom();

    // during the room-absorbing process, some stored objects may be transferred to/from it
    SyncStorageTilesCacheAfterRoomReconfigure();

    // process storage tiles
    {  
        cxx::temp_vector<Point2D> evaluationResult;
        EvaluateStorageTiles(evaluationResult);
        HandleRoomStorageTiles(evaluationResult);
    }
}

int StorageRoomController::GetStoredObjectsCount()
{
    return GetRoom().GetStorageSlots().size();
}

int StorageRoomController::GetStoredObjectsCapacity()
{
    int maxCount = std::max(GetStoredObjectsCount(), mMaxObjectsPerStorageTile * GetStorageTilesCount());
    return maxCount;
}

void StorageRoomController::OnRecycle()
{
    RoomController::OnRecycle();

    mStorageTiles.clear();
}

int StorageRoomController::GetRoomStorageTileIndex(const Point2D& tileLocation) const
{
    const int itemIndex = cxx::get_first_index_if(mStorageTiles, [&tileLocation](const RoomStorageTile& storageTile)
        {
            return (storageTile.mTileLocation == tileLocation);
        });
    return itemIndex;
}

StorageRoomController::RoomStorageTile* StorageRoomController::GetRoomStorageTileFromLocation(const Point2D& tileLocation)
{
    for (RoomStorageTile& roller: mStorageTiles)
    {
        if (roller.mTileLocation == tileLocation) return &roller;
    }
    return nullptr;
}

StorageRoomController::RoomStorageTile* StorageRoomController::GetRoomStorageTileWithObject(EntityHandle entityHandle)
{
    for (RoomStorageTile& roller: mStorageTiles)
    {
        if (roller.mObjects.empty()) continue;

        if (cxx::contains_if(roller.mObjects, [&entityHandle](const EntityHandle& ent) { return (ent == entityHandle); }))
        {
            return &roller;
        }
    }
    return nullptr;
}

StorageRoomController::RoomStorageTile* StorageRoomController::GetFirstAvailableStorageTile()
{
    for (RoomStorageTile& roller: mStorageTiles)
    {
        if (roller.mObjects.size() < mMaxObjectsPerStorageTile) 
            return &roller;
    }
    return nullptr;
}

bool StorageRoomController::AssignObjectToStorageTile(EntityHandle entityHandle, RoomStorageTile& storageTile)
{
    if (!entityHandle.IsGameObject())
    {
        cxx_assert(false);
        return false;
    }

    if (storageTile.mObjects.size() > mMaxObjectsPerStorageTile)
    {
        cxx_assert(false);
        return false;
    }

    // check on storage
    for (const RoomStorageSlot& roller: GetRoom().GetStorageSlots())
    {
        if (roller.mObjectHandle == entityHandle)
        {
            cxx_assert(false);
            return false;
        }
    }

    GameObject* objectInstance = gGameObjectManager.GetObjectPtr(entityHandle);
    cxx_assert(objectInstance);
    if (objectInstance == nullptr) return false;
    if (objectInstance->GetParentRoom())
    {
        cxx_assert(false);
        return false;
    }

    storageTile.mObjects.push_back(entityHandle);
    GetRoom().AssignObjectToStorageSlot(storageTile.mTileLocation, entityHandle);

    return true;
}

bool StorageRoomController::UnassignObjectFromStorage(EntityHandle entityHandle)
{
    if (!entityHandle) return false;

    bool isSuccess = false;
    if (RoomStorageTile* storageTile = GetRoomStorageTileWithObject(entityHandle))
    {
        if (!cxx::erase(storageTile->mObjects, entityHandle))
        {
            cxx_assert(false);
        }
        GetRoom().UnassignStorageSlotObject(storageTile->mTileLocation, entityHandle);
        isSuccess = true;
    }
    cxx_assert(isSuccess);
    return isSuccess;
}

void StorageRoomController::SyncStorageTilesCacheAfterRoomReconfigure()
{
    mStorageTiles.clear();

    for (const RoomStorageSlot& rollerSlot: GetRoom().GetStorageSlots())
    {
        int storageTileIndex = GetRoomStorageTileIndex(rollerSlot.mTileLocation);
        if (storageTileIndex == -1)
        {
            storageTileIndex = static_cast<int>(mStorageTiles.size());
            // append new tile
            RoomStorageTile& storageTile = mStorageTiles.emplace_back();
            storageTile.mTileLocation = rollerSlot.mTileLocation;
        }
        // append object
        RoomStorageTile& storageTile = mStorageTiles[storageTileIndex];
        cxx_assert(storageTile.mObjects.size() <= mMaxObjectsPerStorageTile);
        storageTile.mObjects.push_back(rollerSlot.mObjectHandle);
    }
}

void StorageRoomController::HandleRoomStorageTiles(cxx::span<Point2D> evaluatedTiles)
{
    cxx::temp_list<RoomStorageTile> removedStorageTiles;
    cxx::temp_list<RoomStorageTile> preservedStorageTiles;
    cxx::temp_list<Point2D> addedStorageTiles;

    // filter new and preserved tiles
    for (const Point2D& rollerLocation: evaluatedTiles)
    {
        int tileIndex = GetRoomStorageTileIndex(rollerLocation);
        if (tileIndex == -1)
        {
            addedStorageTiles.push_back(rollerLocation);
        }
        else
        {
            preservedStorageTiles.push_back(mStorageTiles[tileIndex]);
            mStorageTiles.erase(mStorageTiles.begin() + tileIndex);
        }
    }

    // move leftover to removed tiles
    if (!mStorageTiles.empty())
    {
        removedStorageTiles.insert(removedStorageTiles.end(), mStorageTiles.begin(), mStorageTiles.end());
        mStorageTiles.clear();
    }

    // restore preserved tiles
    cxx_assert(mStorageTiles.empty());
    if (!preservedStorageTiles.empty())
    {
        mStorageTiles.insert(mStorageTiles.end(), preservedStorageTiles.begin(), preservedStorageTiles.end());
        preservedStorageTiles.clear();
    }

    // add new tiles
    if (!addedStorageTiles.empty())
    {
        for (const Point2D& rollerLocation: addedStorageTiles)
        {
            RoomStorageTile& storageTile = mStorageTiles.emplace_back();
            storageTile.mTileLocation = rollerLocation;
            storageTile.mObjects.clear(); // make sure it is empty
        }
        addedStorageTiles.clear();
    }

    auto CheckTileWithinRoom = [this](const Point2D& tileLocation)
        {
            return cxx::contains_if(this->GetRoom().GetFloorTiles(), [tileLocation](MapTile* mapTile)
                {
                    return (mapTile->mLocation == tileLocation);
                });
        };

    // handle objects on removed tiles
    for (const RoomStorageTile& rollerTile: removedStorageTiles)
    {
        for (const EntityHandle& objectHandle: rollerTile.mObjects)
        {         
            // if tile is still part of the room then try to reassign storage tile
            if (CheckTileWithinRoom(rollerTile.mTileLocation))
            {
                // find storage tile to place object
                if (RoomStorageTile* storageTile = GetFirstAvailableStorageTile())
                {
                    storageTile->mObjects.push_back(objectHandle);
                    GetRoom().ReassignStorageSlotObject(rollerTile.mTileLocation, objectHandle, storageTile->mTileLocation);
                    // notify
                    StoredObjectReassigned(objectHandle, *storageTile);
                    continue;
                }
            }
            // remove from room entirely
            GetRoom().UnassignStorageSlotObject(rollerTile.mTileLocation, objectHandle);
            // notify
            StoredObjectUnassigned(objectHandle);
            continue;
        }
    }
    removedStorageTiles.clear();
}

