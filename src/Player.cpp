#include "stdafx.h"
#include "Player.h"
#include "RoomManager.h"
#include "GameSession.h"

void Player::Configure(ePlayerID playerID, ePlayerType playerType, const std::string_view& playerName)
{
    Cleanup();

    mPlayerID = playerID;
    mPlayerType = playerType;
    mPlayerName = playerName;
}

void Player::Cleanup()
{
    mPlayerID = {};
    mPlayerType = {};
    mStartCameraTilePos = {};

    mPlayerName.clear();
    mRoomsOwned.clear();
    mRoomsByType.clear();
    mMoneyStorageRooms.clear();

    mIsDefeated = false;

    // reset resources

    for (auto& roller: mResourceAmount)
    {
        roller = 0;
    }

    for (auto& roller: mStartingResourceAmount)
    {
        roller = 0;
    }
}

void Player::SetStartingResourceAmount(eGameResource resourceType, long resourceAmount)
{
    cxx_assert(resourceType < eGameResource_COUNT);
    cxx_assert(resourceAmount >= 0);
    mStartingResourceAmount[resourceType] = resourceAmount;
}

void Player::SetStartCameraTilePosition(const MapPoint2D& tileLocation)
{
    mStartCameraTilePos = tileLocation;
}

void Player::AddToInventory(EntityHandle entity)
{
    if (entity.IsRoom())
    {
        if (cxx::contains(mRoomsOwned, entity))
        {
            cxx_assert(false);
            return;
        }

        if (Room* roomInstance = GetRoomManager().GetRoomPtr(entity))
        {
            cxx_assert(roomInstance->GetOwnerID() == GetPlayerID());

            RoomDefinition* definition = roomInstance->GetDefinition();
            mRoomsOwned.push_back(entity);
            mRoomsByType[definition->mRoomType].push_back(entity);

            // money storage?
            if (roomInstance->HasCapability<MoneyStorageRoomCapability>())
            {
                mMoneyStorageRooms.push_back(entity);
            }
        }
        else
        {
            cxx_assert(false);
        }
        return;
    }
}

void Player::RemoveFromInventory(EntityHandle entity)
{
    if (entity.IsRoom())
    {
        if (!cxx::erase(mRoomsOwned, entity))
        {
            cxx_assert(false);
            return;
        }

        for (auto& roller: mRoomsByType)
        {
            EntitiesList& entitiesList = roller.second;
            if (cxx::erase(entitiesList, entity))
                break;
        }

        cxx::erase(mMoneyStorageRooms, entity);
        return;
    }
}

bool Player::HasOwnedRoomsOfType(RoomTypeId roomType) const
{
    auto it = mRoomsByType.find(roomType);
    if (it != mRoomsByType.end())
    {
        const EntitiesList& entitiesList = it->second;
        return !entitiesList.empty();
    }
    return false;
}

EntityHandle Player::GetFirstOwnedRoomOfType(RoomTypeId roomType) const
{
    auto it = mRoomsByType.find(roomType);
    if (it != mRoomsByType.end())
    {
        const EntitiesList& entitiesList = it->second;
        if (!entitiesList.empty())
            return entitiesList.front();
    }
    return {};
}

EntityHandle Player::GetNextOwnedRoomOfType(RoomTypeId roomType, EntityHandle prevHandle) const
{
    auto it = mRoomsByType.find(roomType);
    if (it != mRoomsByType.end())
    {
        const EntitiesList& entitiesList = it->second;
        if (!entitiesList.empty())
        {
            int index = cxx::get_item_index(entitiesList, prevHandle);
            if (index == -1)
                return entitiesList.back();

            int nextIndex = (index + 1) % entitiesList.size();
            return entitiesList[nextIndex];
        }
    }
    return {};
}

EntityHandle Player::GetLastOwnedRoomOfType(RoomTypeId roomType) const
{
    auto it = mRoomsByType.find(roomType);
    if (it != mRoomsByType.end())
    {
        const EntitiesList& entitiesList = it->second;
        if (!entitiesList.empty())
            return entitiesList.back();
    }
    return {};
}

bool Player::CanBuildRoomOfType(RoomTypeId roomType) const
{
    if (roomType == RoomTypeId_Null) return false;

    if (RoomDefinition* roomDefinition = GetGameSession().GetScenarioDefinition().GetRoomDefinition(roomType))
    {
        if (roomDefinition->mBuildable) return true;
    }
    return false;
}

void Player::ChangeResourceAmount(eGameResource resourceType, long deltaAmount)
{
    cxx_assert(resourceType < eGameResource_COUNT);
    if (deltaAmount != 0)
    {
        const long currAmount = GetResourceAmount(resourceType);
        SetResourceAmount(resourceType, currAmount + deltaAmount);
    }
}

void Player::SetResourceAmount(eGameResource resourceType, long resourceAmount)
{
    cxx_assert(resourceType < eGameResource_COUNT);
    cxx_assert(resourceAmount >= 0);
    mResourceAmount[resourceType] = std::max(resourceAmount, 0L);
}

void Player::SetDefeated()
{
    mIsDefeated = true;
}
