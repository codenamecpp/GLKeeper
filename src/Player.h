#pragma once

//////////////////////////////////////////////////////////////////////////

#include "PlayerDefs.h"
#include "RoomDefs.h"

//////////////////////////////////////////////////////////////////////////

class Player final: public cxx::noncopyable
{
public:
    void Configure(ePlayerID playerId, ePlayerType playerType, const std::string_view& playerName);
    void Cleanup();

    void SetStartingResourceAmount(eGameResource resourceType, long resourceAmount);
    long GetStartingResourceAmount(eGameResource resourceType) const
    {
        cxx_assert(resourceType < eGameResource_COUNT);
        return mStartingResourceAmount[resourceType];
    }

    void SetStartCameraTilePosition(const MapPoint2D& tileLocation);
    const MapPoint2D& GetStartCameraTilePosition() const 
    { 
        return mStartCameraTilePos; 
    }

    // get player identifier, its value is constant during gameplay session
    inline ePlayerID GetPlayerId() const { return mPlayerId; }
    inline bool IsPlayerId(ePlayerID playerId) const
    {
        return mPlayerId == playerId;
    }

    // get player type, its value is constant during gameplay session
    inline ePlayerType GetPlayerType() const { return mPlayerType; }
    inline bool IsPlayerType(ePlayerType playerType) const
    {
        return mPlayerType == playerType;
    }

    // manage player's inventory:
    // rooms, objects, etc.
    void AddToInventory(EntityHandle entity);
    void RemoveFromInventory(EntityHandle entity);

    // accessing owned rooms
    EntityHandle GetFirstOwnedRoomOfType(RoomTypeId roomType) const;
    EntityHandle GetNextOwnedRoomOfType(RoomTypeId roomType, EntityHandle prevHandle) const;
    EntityHandle GetLastOwnedRoomOfType(RoomTypeId roomType) const;
    inline cxx::span<const EntityHandle> GetOwnedMoneyStorageRooms() const { return mMoneyStorageRooms; }
    inline cxx::span<const EntityHandle> GetOwnedRoomOfType(RoomTypeId roomType) const
    {
        auto it = mRoomsByType.find(roomType);
        if (it != mRoomsByType.end())
            return it->second;

        return {};
    }
    bool HasOwnedRoomsOfType(RoomTypeId roomType) const;

    // tech availability
    bool CanBuildRoomOfType(RoomTypeId roomType) const;

    // resource management
    void ChangeResourceAmount(eGameResource resourceType, long deltaAmount);
    void SetResourceAmount(eGameResource resourceType, long resourceAmount);
    inline long GetResourceAmount(eGameResource resourceType) const
    {
        cxx_assert(resourceType < eGameResource_COUNT);
        return mResourceAmount[resourceType];
    }

    // current defeated state
    void SetDefeated();
    inline bool IsDefeated() const
    {
        return (mPlayerType != ePlayerType_Null) &&  mIsDefeated;
    }
    inline bool IsAlive() const
    {
        return (mPlayerType != ePlayerType_Null) && !mIsDefeated;
    }
    // player type shortcuts
    inline bool IsNonPlayer () const { return IsPlayerType(ePlayerType_Null); }
    inline bool IsHuman     () const { return IsPlayerType(ePlayerType_Human); }
    inline bool IsComputer  () const { return IsPlayerType(ePlayerType_AI); }

    inline bool IsKeeperPlayer() const
    {
        return (mPlayerType != ePlayerType_Null) && (mPlayerId >= ePlayerID_Keeper1);
    }

private:
    using EntitiesList = std::vector<EntityHandle>;

    // properties
    ePlayerID mPlayerId = ePlayerID_Null;
    ePlayerType mPlayerType = ePlayerType_Null;
    std::string mPlayerName;

    // start config
    MapPoint2D mStartCameraTilePos;

    // inventory
    EntitiesList mRoomsOwned;
    EntitiesList mMoneyStorageRooms;
    std::map<RoomTypeId, EntitiesList> mRoomsByType;

    long mResourceAmount[eGameResource_COUNT]; // cache
    long mStartingResourceAmount[eGameResource_COUNT];

    // state
    bool mIsDefeated = false;
};

//////////////////////////////////////////////////////////////////////////