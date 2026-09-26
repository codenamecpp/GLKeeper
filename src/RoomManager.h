#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameDefs.h"
#include "ScenarioDefs.h"
#include "Room.h"
#include "RoomController.h"
#include "EntityDefs.h"

//////////////////////////////////////////////////////////////////////////

class RoomManager final: public cxx::noncopyable
{
private:

    //////////////////////////////////////////////////////////////////////////
    struct RoomInstanceSlot { uint32_t mGeneration = 1; 
        cxx::uniqueptr<Room> mInstance; 
        cxx::uniqueptr<RoomController> mController;
    };
    //////////////////////////////////////////////////////////////////////////

public:

    bool LoadScenario(const ScenarioDefinition& scenarioDef);
    void EnterWorld();
    void ClearWorld();

    void UpdateFrame(float deltaTime);
    void UpdateLogic(float stepDeltaTime);

    // forcibly refresh active room lists, destroy rooms pending destruction
    // note: this will invalidate list iterators
    void ProcessRoomChanges();

    EntityHandle CreateScenarioRoom(const ScenarioRoomThing& roomThing);
    EntityHandle CreateRoom(RoomTypeId typeId, ePlayerID ownerId, eDirection direction = eDirection_N);
    EntityHandle CreateRoom(RoomDefinition* definition, ePlayerID ownerId, eDirection direction = eDirection_N);

    EntityHandle FindRoom(EntityUid instanceUid) const;

    Room* GetRoomPtr(EntityUid instanceUid) const;
    Room* GetRoomPtr(const EntityHandle& roomHandle) const;

    bool ActivateRoom(const EntityHandle& roomHandle);
    bool ActivateRoom(EntityUid instanceUid);

    bool DeleteRoom(const EntityHandle& roomHandle);
    bool DeleteRoom(EntityUid instanceUid);

    // check whether the room was spawned and remains active on map
    bool ExistsOnMap(const EntityHandle& roomHandle) const;

    // get all currently active rooms in game world
    // warning: do not store result
    inline cxx::span<Room*> GetRooms() const { return mActiveRooms; }
    inline cxx::span<Room*> GetRoomsByType(RoomTypeId roomTypeId) const
    {
        auto map_it = mActiveRoomsByType.find(roomTypeId);
        if (map_it != mActiveRoomsByType.end())
        {
            return map_it->second;
        }
        return {};
    }

    // get number of currently active rooms in game world
    inline int GetRoomCount() const { return static_cast<int>(mActiveRooms.size()); }

private:
    // factory
    cxx::uniqueptr<Room> NewRoomInstance() const;

    template<typename TRoomController>
    cxx::uniqueptr<RoomController> NewControllerInstance() const;
    cxx::uniqueptr<RoomController> NewControllerInstance(RoomDefinition* roomDefinition) const;

    void ProcessRegistrationQueue();
    void ProcessRemoveQueue();

    void DestroyRooms();

    void ConfigureNewRoomInstance(Room* roomInstance, 
        RoomController* controller, 
        RoomDefinition* definition, 
        EntityUid instanceUid, eDirection roomDirection, ePlayerID ownerId);

    void RegisterRoom(Room* roomInstance);
    void UnregisterRoom(Room* roomInstance);

private:
    std::vector<RoomInstanceSlot> mRoomSlots;
    std::unordered_map<EntityUid, EntityHandle> mInstanceUidsMap;
    std::unordered_map<RoomTypeId, std::vector<Room*>> mActiveRoomsByType;

    std::vector<Room*> mActiveRooms;
    std::vector<EntityHandle> mRegistrationQueue; // pending registration in lists
    std::vector<EntityHandle> mRemoveQueue; // pending destroy
};

//////////////////////////////////////////////////////////////////////////

extern RoomManager gRoomManager;

//////////////////////////////////////////////////////////////////////////