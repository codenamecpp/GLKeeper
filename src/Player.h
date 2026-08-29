#pragma once

//////////////////////////////////////////////////////////////////////////

#include "PlayerDefs.h"
#include "RoomDefs.h"

//////////////////////////////////////////////////////////////////////////

class Player final: public cxx::noncopyable
{
public:
    void Configure(ePlayerID playerId, ePlayerType playerType, std::string_view playerName);
    void Cleanup();

    void SetStartingResourceAmount(eGameResource resourceType, long resourceAmount);
    long GetStartingResourceAmount(eGameResource resourceType) const
    {
        cxx_assert(resourceType < eGameResource_COUNT);
        return mStartingResourceAmount[resourceType];
    }

    void SetStartCameraTilePosition(const Point2D& tileLocation);
    const Point2D& GetStartCameraTilePosition() const 
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
    // owned rooms, objects, etc.
    void AddToInventory(EntityHandle entity);
    void RemoveFromInventory(EntityHandle entity);

    // accessing owned rooms
    EntityHandle GetFirstOwnedRoomOfType(RoomTypeId roomType) const;
    EntityHandle GetNextOwnedRoomOfType(RoomTypeId roomType, EntityHandle prevHandle) const;
    EntityHandle GetLastOwnedRoomOfType(RoomTypeId roomType) const;
    const auto& GetOwnedMoneyStorageRooms() const { return mMoneyStorageRooms; }
    const auto& GetOwnedRoomOfType(RoomTypeId roomType) const
    {
        static const std::vector<EntityHandle> nullresult;
        auto it = mRoomsByType.find(roomType);
        return (it != mRoomsByType.end()) ? it->second : nullresult;
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

    //////////////////////////////////////////////////////////////////////////

    // in hand

    void AddHeldEntity(EntityHandle entity);
    void RemoveHeldEntity(EntityHandle entity);
    bool HasSomethingInHand() const;
    bool HasInHand(EntityHandle entity) const;

    EntityHandle GetFirstHeldEntity() const;
    EntityHandle GetNextHeldEntity(EntityHandle prevHandle) const;
    EntityHandle GetLastHeldEntity() const;

    inline const auto& GetHeldEntities() const { return mHeldEntities; }

    //////////////////////////////////////////////////////////////////////////

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
    // properties
    ePlayerID mPlayerId = ePlayerID_Null;
    ePlayerType mPlayerType = ePlayerType_Null;
    std::string mPlayerName;

    // start config
    Point2D mStartCameraTilePos;

    // inventory
    std::vector<EntityHandle> mRoomsOwned;
    std::vector<EntityHandle> mMoneyStorageRooms;
    std::map<RoomTypeId, std::vector<EntityHandle>> mRoomsByType;

    std::vector<EntityHandle> mHeldEntities;

    long mResourceAmount[eGameResource_COUNT]; // cache
    long mStartingResourceAmount[eGameResource_COUNT];

    // state
    bool mIsDefeated = false;
};

//////////////////////////////////////////////////////////////////////////