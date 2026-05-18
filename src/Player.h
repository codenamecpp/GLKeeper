#pragma once

//////////////////////////////////////////////////////////////////////////

#include "PlayerDefs.h"
#include "RoomDefs.h"
#include "GameSessionAware.h"

//////////////////////////////////////////////////////////////////////////

class Player final: private GameSessionAware
{
public:
    void Configure(ePlayerID playerID, ePlayerType playerType, const std::string_view& playerName);
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
    inline ePlayerID GetPlayerID() const { return mPlayerID; }

    // get player type, its value is constant during gameplay session
    inline ePlayerType GetPlayerType() const { return mPlayerType; }

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
    inline bool IsNonPlayer () const { return mPlayerType == ePlayerType_Null; }
    inline bool IsHuman     () const { return mPlayerType == ePlayerType_Human; }
    inline bool IsComputer  () const { return mPlayerType == ePlayerType_AI; }

private:
    using EntitiesList = std::vector<EntityHandle>;

    // properties
    ePlayerID   mPlayerID = ePlayerID_Null;
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