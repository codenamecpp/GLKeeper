#include "stdafx.h"
#include "Player.h"
#include "RoomManager.h"
#include "GameSession.h"

void Player::Configure(ePlayerID playerId, ePlayerType playerType, std::string_view playerName)
{
    Cleanup();

    mPlayerId = playerId;
    mPlayerType = playerType;
    mPlayerName = playerName;
}

void Player::Cleanup()
{
    mPlayerId = {};
    mPlayerType = {};
    mStartCameraTilePos = {};

    mPlayerName.clear();
    mRoomsOwned.clear();
    mRoomsByType.clear();
    mMoneyStorageRooms.clear();
    mHeldEntities.clear();

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

void Player::SetStartCameraTilePosition(const Point2D& tileLocation)
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

        if (Room* roomInstance = gRoomManager.GetRoomPtr(entity))
        {
            cxx_assert(roomInstance->GetOwnerId() == GetPlayerId());

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
            std::vector<EntityHandle>& entitiesList = roller.second;
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
        const std::vector<EntityHandle>& entitiesList = it->second;
        return !entitiesList.empty();
    }
    return false;
}

EntityHandle Player::GetFirstOwnedRoomOfType(RoomTypeId roomType) const
{
    auto it = mRoomsByType.find(roomType);
    if (it != mRoomsByType.end())
    {
        const std::vector<EntityHandle>& entitiesList = it->second;
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
        const std::vector<EntityHandle>& entitiesList = it->second;
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
        const std::vector<EntityHandle>& entitiesList = it->second;
        if (!entitiesList.empty())
            return entitiesList.back();
    }
    return {};
}

bool Player::CanBuildRoomOfType(RoomDefinition* roomDefinition) const
{
    cxx_assert(roomDefinition);
    return (roomDefinition && roomDefinition->mBuildable);
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

void Player::AddHeldEntity(EntityHandle entity)
{
    if (!entity.IsCreature() && !entity.IsGameObject())
    {
        cxx_assert(false);
        return;
    }

    if (!cxx::contains(mHeldEntities, entity))
    {
        mHeldEntities.push_back(entity);
    }
}

void Player::RemoveHeldEntity(EntityHandle entity)
{
    cxx::erase(mHeldEntities, entity);
}

EntityHandle Player::GetFirstHeldEntity() const
{
    return !mHeldEntities.empty() ? mHeldEntities.front() : EntityHandle {};
}

EntityHandle Player::GetNextHeldEntity(EntityHandle prevHandle) const
{
    if (!mHeldEntities.empty())
    {
        int index = cxx::get_item_index(mHeldEntities, prevHandle);
        if (index == -1)
        {
            return mHeldEntities.back();
        }
        int nextIndex = (index + 1) % mHeldEntities.size();
        return mHeldEntities[nextIndex];
    }
    return EntityHandle {};
}

EntityHandle Player::GetLastHeldEntity() const
{
    return !mHeldEntities.empty() ? mHeldEntities.back() : EntityHandle {};
}

bool Player::HasInHand(EntityHandle entity) const
{
    return cxx::contains(mHeldEntities, entity);
}

bool Player::HasSomethingInHand() const
{
    return !mHeldEntities.empty();
}

const auto& Player::GetOwnedRoomOfType(RoomTypeId roomType) const
{
    static const std::vector<EntityHandle> nullresult;
    auto it = mRoomsByType.find(roomType);
    return (it != mRoomsByType.end()) ? it->second : nullresult;
}
