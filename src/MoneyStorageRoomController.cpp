#include "stdafx.h"
#include "MoneyStorageRoomController.h"
#include "GameObjectManager.h"
#include "EconomyService.h"
#include "MapUtils.h"

//////////////////////////////////////////////////////////////////////////

static const long MoneyStorageDefaultMaxGoldPerTile = 1000;

//////////////////////////////////////////////////////////////////////////

MoneyStorageRoomController::MoneyStorageRoomController()
    : StorageRoomController(1)
    , mMoneyStorageMaxGoldPerTile(MoneyStorageDefaultMaxGoldPerTile)
{
}

void MoneyStorageRoomController::ConfigureInstance(Room* roomInstance)
{
    StorageRoomController::ConfigureInstance(roomInstance);

    // wire components
    mMoneyComponent = GetRoom().GetComponent<MoneyComponent>();
    cxx_assert(mMoneyComponent);

    // setup capabilities
    GetRoom().SetCapability<MoneyStorageRoomCapability>(this);
}

void MoneyStorageRoomController::SpawnInstance()
{
    StorageRoomController::SpawnInstance();

    SyncStoredMoneyAmount();
}

void MoneyStorageRoomController::DespawnInstance()
{
    StorageRoomController::DespawnInstance();
}

void MoneyStorageRoomController::PostRearrangeObjects()
{
    StorageRoomController::PostRearrangeObjects();
}

void MoneyStorageRoomController::PostReconfigureRoom()
{
    StorageRoomController::PostReconfigureRoom();

    // update max capacity
    if (mMoneyComponent)
    {
        mMoneyComponent->mCapacity = mMoneyStorageMaxGoldPerTile * GetStorageTilesCount();
    }

    // update stored gold amount
    SyncStoredMoneyAmount();

    // auto add loose gold to storage
    ScanForLooseGold();
}

void MoneyStorageRoomController::UpdateLogic(float stepDeltaTime)
{
    StorageRoomController::UpdateLogic(stepDeltaTime);
    
    ScanForLooseGold(); // todo: optimize
}

void MoneyStorageRoomController::RoomOwnershipChanged(ePlayerID previousOwnerId, ePlayerID ownerId)
{
    StorageRoomController::RoomOwnershipChanged(previousOwnerId, ownerId);

    cxx_assert(previousOwnerId != ownerId);

    if (mMoneyComponent)
    {
        const long moneyToTransfer = mMoneyComponent->mAmount;
        gEconomyService.StoredMoneyAmountChanged(previousOwnerId, GetRoom().GetOwnHandle(), -moneyToTransfer);
        gEconomyService.StoredMoneyAmountChanged(ownerId, GetRoom().GetOwnHandle(), moneyToTransfer);
    }
}

void MoneyStorageRoomController::OnRecycle()
{
    StorageRoomController::OnRecycle();
    mMoneyComponent = nullptr;
    mMoneyStorageMaxGoldPerTile = MoneyStorageDefaultMaxGoldPerTile;
}

long MoneyStorageRoomController::GetStoredGoldAmount() const
{
    return mMoneyComponent->mAmount;
}

long MoneyStorageRoomController::GetStoredGoldCapacity() const
{
    return mMoneyComponent->mCapacity;
}

long MoneyStorageRoomController::StoreGold(long goldAmount)
{
    if ((goldAmount < 1) || (GetFreeStorageSpace() == 0)) 
        return 0;

    long distributedAmount = DistributeGoldBetweenChests(goldAmount, true);
    // update stored gold amount
    if (distributedAmount > 0)
    {
        SyncStoredMoneyAmount();
    }
    return distributedAmount;
}

long MoneyStorageRoomController::StoreGold(long goldAmount, const Point2D& tileLocation)
{
    if ((goldAmount < 1) || (GetFreeStorageSpace() == 0)) 
        return 0;

    long distributedAmount = StoreGoldOnStorageTile(tileLocation, goldAmount, true);
    if (distributedAmount < goldAmount)
    {
        distributedAmount += DistributeGoldBetweenChests(goldAmount - distributedAmount, true);
    }
    if (distributedAmount > 0)
    {
        SyncStoredMoneyAmount();
    }
    return distributedAmount;
}

long MoneyStorageRoomController::DisposeGold(long goldAmount)
{
    if ((goldAmount < 0) || (GetStoredGoldAmount() == 0)) 
        return 0;

    cxx::temp_list<EntityHandle> releaseObjects;

    long resultAmount = 0;
    // dispose gold from chests
    for (const RoomStorageSlot& storageSlot: GetRoom().GetStorageSlots())
    {
        GameObject* objectInstance = gGameObjectManager.GetObjectPtr(storageSlot.mObjectHandle);
        cxx_assert(objectInstance);
        if (objectInstance == nullptr) 
            continue;

        auto* storageCapability = objectInstance->GetCapability<GoldContainerCapability>();
        if (storageCapability == nullptr) 
            continue;

        // trying use chest
        long removedFromChest = storageCapability->DisposeGold(goldAmount);
        if (removedFromChest > 0)
        {
            resultAmount += removedFromChest;
            goldAmount -= removedFromChest;
        }
        // release chest
        if (storageCapability->GetStoredGoldAmount() == 0)
        {
            releaseObjects.push_back(storageSlot.mObjectHandle);
        }
        // done?
        if (goldAmount <= 0) 
            break;
    }

    // release objects
    for (const EntityHandle& roller: releaseObjects)
    {
        if (!UnassignObjectFromStorage(roller))
        {
            cxx_assert(false);
        }
        gGameObjectManager.DeleteObject(roller);
    }

    // update stored gold amount
    if (resultAmount > 0)
    {
        SyncStoredMoneyAmount();
    }
    return resultAmount;
}

bool MoneyStorageRoomController::GetTileToStoreGold(Point2D& tileLocation)
{
    if (GetFreeStorageSpace() == 0)
        return false;

    for (const RoomStorageTile& storageRoller: GetStorageTiles())
    {
        if (storageRoller.mObjects.size() < GetMaxObjectsPerStorageTile())
        {
            tileLocation = storageRoller.mTileLocation;
            return true;
        }

        for (EntityHandle objectsRoller: storageRoller.mObjects)
        {
            GameObject* objectInstance = gGameObjectManager.GetObjectPtr(objectsRoller);
            cxx_assert(objectInstance);
            if (objectInstance == nullptr) 
            {
                continue;
            }
            auto* storageCapability = objectInstance->GetCapability<GoldContainerCapability>();
            cxx_assert(storageCapability);
            if (storageCapability == nullptr)
            {
                continue;
            }
            long storageCapacity = storageCapability->GetStoredGoldCapacity();
            if ((storageCapacity == 0) || 
                (storageCapacity > storageCapability->GetStoredGoldAmount()))
            {
                tileLocation = storageRoller.mTileLocation;
                return true;
            }   
        }
    }
    return false;
}

long MoneyStorageRoomController::GetFreeStorageSpace() const
{
    long freeCapacity = std::max(mMoneyComponent->mCapacity - mMoneyComponent->mAmount, 0L);
    return freeCapacity;
}

void MoneyStorageRoomController::StoredObjectUnassigned(EntityHandle entityHandle)
{  
    // delete chest and create gold pile

    GameObject* gameObject = gGameObjectManager.GetObjectPtr(entityHandle);
    if (gameObject == nullptr) 
        return;

    if (gameObject->GetClassId() == GameObjectClassId_GoldPile) 
        return;

    if (auto* goldContainer = gameObject->GetCapability<GoldContainerCapability>())
    {
        long goldAmount = goldContainer->GetStoredGoldAmount();
        if (goldAmount > 0)
        {
            EntityHandle newEntity = gGameObjectManager.CreateGoldPile(goldAmount);

            SetObjectPlacementToTileCenter(newEntity, gameObject->GetTilePosition(), true);
            if (!gGameObjectManager.ActivateObject(newEntity))
            {
                cxx_assert(false);
            }
        }
    }
    gGameObjectManager.DeleteObject(entityHandle);
}

void MoneyStorageRoomController::StoredObjectReassigned(EntityHandle entityHandle, const RoomStorageTile& newStorageTile)
{
    SetObjectPlacementToTileCenter(entityHandle, newStorageTile.mTileLocation, false);
}

void MoneyStorageRoomController::StoredObjectAssigned(EntityHandle entityHandle, const RoomStorageTile& newStorageTile)
{
    SetObjectPlacementToTileCenter(entityHandle, newStorageTile.mTileLocation, true);
}

void MoneyStorageRoomController::SetMoneyStorageMaxGoldPerTile(long maxGoldPerTile)
{
    mMoneyStorageMaxGoldPerTile = maxGoldPerTile;
    cxx_assert(mMoneyStorageMaxGoldPerTile > 0);
}

void MoneyStorageRoomController::SetObjectPlacementToTileCenter(EntityHandle entityHandle, const Point2D& tileLocation, bool setRandomOrientation)
{
    GameObject* gameObject = gGameObjectManager.GetObjectPtr(entityHandle);
    cxx_assert(gameObject);

    if (gameObject == nullptr) 
        return;

    glm::vec3 objectPosition = MapUtils::ComputeTileCenter(tileLocation);
    gameObject->SetPosition(objectPosition);
    if (setRandomOrientation)
    {
        gameObject->SetOrientation(Random::GenerateAngle());
    }
    gameObject->SnapPositionToFloor();
}

void MoneyStorageRoomController::SyncStoredMoneyAmount()
{
    if (mMoneyComponent == nullptr) 
        return;

    long currentGoldAmount = 0;
    GameObjectManager& gobjects = gGameObjectManager;
    for (const RoomStorageSlot& storageSlot: GetRoom().GetStorageSlots())
    {
        GameObject* gameObject = gobjects.GetObjectPtr(storageSlot.mObjectHandle);
        if (gameObject == nullptr) 
        {
            cxx_assert(false);
            continue;
        }
        if (auto* goldContainer = gameObject->GetCapability<GoldContainerCapability>())
        {
            currentGoldAmount += goldContainer->GetStoredGoldAmount();
        }
    }

    if (mMoneyComponent->mAmount != currentGoldAmount)
    {
        long deltaAmount = currentGoldAmount - mMoneyComponent->mAmount;
        mMoneyComponent->mAmount = currentGoldAmount;
        gEconomyService.StoredMoneyAmountChanged(GetRoom().GetOwnerId(), GetRoom().GetOwnHandle(), deltaAmount);
    }
}

void MoneyStorageRoomController::ScanForLooseGold()
{
    if ((GetStorageTilesCount() == 0) || (GetFreeStorageSpace() == 0)) 
        return;

    long addedLooseGoldAmount = 0;

    for (GameObject* gameObject: gGameObjectManager.GetGoldContainers())
    {
        // ignore inactive / owned
        if (!gameObject->ExistsOnMap() || gameObject->GetParentRoom().IsRoom()) 
            continue;

        // whether object is on one of storage tiles
        Point2D mapLocation = gameObject->GetTilePosition();

        RoomStorageTile* storageTile = GetRoomStorageTileFromLocation(mapLocation);
        if (storageTile == nullptr)
            continue;

        auto* goldContainer = gameObject->GetCapability<GoldContainerCapability>();
        cxx_assert(goldContainer);
        
        // try store on target tile first
        long storedAmount = StoreGoldOnStorageTile(*storageTile, goldContainer->GetStoredGoldAmount(), true);
        if (storedAmount > 0)
        {
            goldContainer->DisposeGold(storedAmount);
            addedLooseGoldAmount += storedAmount;
        }
        // store on any tile
        if (goldContainer->GetStoredGoldAmount() > 0)
        {
            long distributedAmount = DistributeGoldBetweenChests(goldContainer->GetStoredGoldAmount(), true);
            if (distributedAmount > 0)
            {
                goldContainer->DisposeGold(distributedAmount);
                addedLooseGoldAmount += distributedAmount;
            }
        }

        // delete empty containers
        if (goldContainer->GetStoredGoldAmount() == 0)
        {
            gGameObjectManager.DeleteObject(gameObject->GetOwnHandle());
        }

        // done?
        if (GetFreeStorageSpace() == 0) 
            break;
    }

    // update stored gold amount
    if (addedLooseGoldAmount > 0)
    {
        SyncStoredMoneyAmount();
    }
}

long MoneyStorageRoomController::StoreGoldOnStorageTile(const Point2D& tileLocation, long goldAmount, bool canCreateChest)
{
    long resultAmount = 0;
    if (RoomStorageTile* storageTile = GetRoomStorageTileFromLocation(tileLocation))
    {
        resultAmount = StoreGoldOnStorageTile(*storageTile, goldAmount, canCreateChest);
    }
    return resultAmount;
}

long MoneyStorageRoomController::StoreGoldOnStorageTile(RoomStorageTile& storageTile, long goldAmount, bool canCreateChest)
{
    long resultAmount = 0;

    cxx_assert(goldAmount > 0);
    if (goldAmount > 0)
    {
        // distribute gold between existing chests
        for (const EntityHandle entHandle: storageTile.mObjects)
        {
            long storedAmount = StoreGoldInContainer(entHandle, goldAmount);
            if (storedAmount > 0)
            {
                resultAmount += storedAmount;
                goldAmount -= storedAmount;
            }
            // done?
            if (goldAmount <= 0) 
                break;
        }

        // create additional chest if needed
        if (canCreateChest)
        {
            while (goldAmount > 0)
            {
                // check free space on storage tile
                bool hasFreeStorageSpace = storageTile.mObjects.size() < GetMaxObjectsPerStorageTile();
                if (!hasFreeStorageSpace) 
                    break;

                long storedAmount = std::min(mMoneyStorageMaxGoldPerTile, goldAmount);
                EntityHandle chestObject = gGameObjectManager.CreateGoldChest(storedAmount, mMoneyStorageMaxGoldPerTile);
                gGameObjectManager.ActivateObject(chestObject);
                if (!chestObject || !AssignObjectToStorageTile(chestObject, storageTile))
                {
                    cxx_assert(false);
                    break;
                }

                SetObjectPlacementToTileCenter(chestObject, storageTile.mTileLocation, true);

                resultAmount += storedAmount;
                goldAmount -= storedAmount;
            }
        }
    }
    return resultAmount;
}

long MoneyStorageRoomController::StoreGoldInContainer(EntityHandle objectHandle, long goldAmount)
{
    GameObject* gameObject = gGameObjectManager.GetObjectPtr(objectHandle);
    cxx_assert(gameObject);
    if (gameObject)
    {
        if (auto* storageCapability = gameObject->GetCapability<GoldContainerCapability>())
        {
            return storageCapability->StoreGold(goldAmount);
        }
    }
    return 0;
}

long MoneyStorageRoomController::DistributeGoldBetweenChests(long goldAmount, bool canCreateAdditionalChests)
{
    long resultAmount = 0;

    // distribute gold between existing chests
    for (const RoomStorageSlot& storageSlot: GetRoom().GetStorageSlots())
    {
        long storedAmount = StoreGoldInContainer(storageSlot.mObjectHandle, goldAmount);
        if (storedAmount > 0)
        {
            resultAmount += storedAmount;
            goldAmount -= storedAmount;
        }
        // done?
        if (goldAmount <= 0) 
            break;
    }

    // create additional chests if needed
    if (canCreateAdditionalChests)
    {
        while (goldAmount > 0)
        {
            RoomStorageTile* storageTile = GetFirstAvailableStorageTile();
            if (storageTile == nullptr) 
                break; // no free storage space

            long storedAmount = StoreGoldOnStorageTile(*storageTile, goldAmount, canCreateAdditionalChests);
            if (storedAmount > 0)
            {
                resultAmount += storedAmount;
                goldAmount -= storedAmount;
            }
        }
    }
    return resultAmount;
}

