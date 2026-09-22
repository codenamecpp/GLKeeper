#pragma once

//////////////////////////////////////////////////////////////////////////

#include "StorageRoomController.h"

//////////////////////////////////////////////////////////////////////////

class MoneyStorageRoomController: public StorageRoomController
    , protected MoneyStorageRoomCapability
{
public:
    MoneyStorageRoomController();

    // override StorageRoomController
    void ConfigureInstance(Room* roomInstance) override;
    void SpawnInstance() override;
    void DespawnInstance() override;
    void PostRearrangeObjects() override;
    void PostReconfigureRoom() override;
    void UpdateLogic(float stepDeltaTime) override;
    void RoomOwnershipChanged(ePlayerID previousOwnerId, ePlayerID ownerId) override;

    // pool
    void OnRecycle() override;

protected:
    // override MoneyStorageRoomCapability
    long GetStoredGoldAmount() const override;
    long GetStoredGoldCapacity() const override;
    long StoreGold(long goldAmount) override;
    long StoreGold(long goldAmount, const Point2D& tileLocation) override;
    long DisposeGold(long goldAmount) override;
    bool GetTileToStoreGold(Point2D& tileLocation) override;
    long GetFreeStorageSpace() const;

    // override StorageRoomController
    void StoredObjectUnassigned(EntityHandle entityHandle) override;
    void StoredObjectReassigned(EntityHandle entityHandle, const RoomStorageTile& newStorageTile) override;
    void StoredObjectAssigned(EntityHandle entityHandle, const RoomStorageTile& newStorageTile) override;

    void SetMoneyStorageMaxGoldPerTile(long maxGoldPerTile);

    void SetObjectPlacementToTileCenter(EntityHandle entityHandle, const Point2D& tileLocation, bool setRandomOrientation);
    void SyncStoredMoneyAmount();
    void ScanForLooseGold();

    long StoreGoldOnStorageTile(const Point2D& tileLocation, long goldAmount, bool canCreateChest);
    long StoreGoldOnStorageTile(RoomStorageTile& storageTile, long goldAmount, bool canCreateChest);
    long StoreGoldInContainer(EntityHandle objectHandle, long goldAmount);
    long DistributeGoldBetweenChests(long goldAmount, bool canCreateAdditionalChests);

protected:
    MoneyComponent* mMoneyComponent = nullptr;

private:
    long mMoneyStorageMaxGoldPerTile;
};

//////////////////////////////////////////////////////////////////////////