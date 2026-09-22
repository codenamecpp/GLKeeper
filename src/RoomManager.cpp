#include "stdafx.h"
#include "RoomManager.h"
#include "Room.h"
#include "HeroGateFrontendRoomController.h"
#include "TempleRoomController.h"
#include "DungeonHeartRoomController.h"
#include "WorkshopRoomController.h"
#include "GameWorld.h"
#include "SimplePool.h"
#include "LibraryRoomController.h"
#include "TortureChamberRoomController.h"
#include "TrainingRoomController.h"
#include "HatcheryRoomController.h"
#include "TreasuryRoomController.h"
#include "GameMain.h"
#include "GameSession.h"

//////////////////////////////////////////////////////////////////////////

RoomManager gRoomManager;

//////////////////////////////////////////////////////////////////////////

bool RoomManager::LoadScenario(const ScenarioDefinition& scenarioDef)
{
    return true;
}

void RoomManager::EnterWorld()
{
    for (Room* roomInstance: mActiveRooms)
    {
        roomInstance->RearrangeObjects();
    }
}

void RoomManager::ClearWorld()
{
    DestroyRooms();
}

void RoomManager::UpdateFrame(float deltaTime)
{
    ProcessRoomChanges();
}

void RoomManager::UpdateLogic(float stepDeltaTime)
{
    // don't use iterators because of new room that may be added during the update
    for (size_t i = 0, MaxUpdateRooms = mActiveRooms.size(); i < MaxUpdateRooms; ++i)
    {
        Room* room = mActiveRooms[i];
        if (room->Exists())
        {
            room->UpdateLogic(stepDeltaTime);
        }
    }
}

void RoomManager::ProcessRoomChanges()
{
    ProcessRegistrationQueue();
    ProcessRemoveQueue();
}

EntityHandle RoomManager::CreateScenarioRoom(const ScenarioRoomThing& roomThing)
{
    EntityHandle roomHandle = CreateRoom(roomThing.mRoomType, roomThing.mPlayerId, roomThing.mDirection);
    
    if (Room* roomInstance = GetRoomPtr(roomHandle))
    {
        // todo: startup params
    }
    return roomHandle;
}

EntityHandle RoomManager::CreateRoom(RoomTypeId typeId, ePlayerID ownerId, eDirection direction)
{
    if (RoomDefinition* roomDefinition = gGameSession.GetScenarioDefinition().GetRoomDefinition(typeId))
    {
        return CreateRoom(roomDefinition, ownerId, direction);
    }
    gConsole.LogMessage(eLogLevel_Warning, "Cannot create room with type id '%d'", typeId);
    cxx_assert(false);
    return {};
}

EntityHandle RoomManager::CreateRoom(RoomDefinition* roomDefinition, ePlayerID ownerId, eDirection direction)
{
    cxx_assert(roomDefinition);
    if (roomDefinition == nullptr) return {}; // nothing to create
    
    int freeSlotIndex = cxx::get_first_index_if(mRoomSlots, [](const RoomInstanceSlot& slot)
        {
            return !slot.mInstance;
        });
    if (freeSlotIndex == -1)
    {
        freeSlotIndex = static_cast<int>(mRoomSlots.size());
        mRoomSlots.emplace_back();
    }

    RoomInstanceSlot& roomSlot = mRoomSlots[freeSlotIndex];
    roomSlot.mInstance = NewRoomInstance();
    roomSlot.mController = NewControllerInstance(roomDefinition);
    cxx_assert(roomSlot.mController);

    const EntityHandle roomHandle { eEntityType_Room, roomSlot.mGeneration, static_cast<uint32_t>(freeSlotIndex) };
    const EntityUid instanceUid = gGameWorld.GenerateEntityUid();
    mInstanceUidsMap[instanceUid] = roomHandle;
    ConfigureNewRoomInstance(roomSlot.mInstance.get(), roomSlot.mController.get(), roomDefinition, instanceUid, direction, ownerId);
    return roomHandle;
}

EntityHandle RoomManager::FindRoom(EntityUid instanceUid) const
{
    auto map_it = mInstanceUidsMap.find(instanceUid);
    if (map_it != mInstanceUidsMap.end())
    {
        return map_it->second;
    }
    return {};
}

Room* RoomManager::GetRoomPtr(EntityUid instanceUid) const
{
    EntityHandle roomHandle = FindRoom(instanceUid);
    return GetRoomPtr(roomHandle);
}

Room* RoomManager::GetRoomPtr(const EntityHandle& roomHandle) const
{
    Room* roomPtr = nullptr;
    if (roomHandle.IsRoom() && (roomHandle.mIndex < mRoomSlots.size()))
    {
        const RoomInstanceSlot& roomSlot = mRoomSlots[roomHandle.mIndex];
        if (roomHandle.mGeneration == roomSlot.mGeneration)
        {
            roomPtr = roomSlot.mInstance.get();
        }
    }
    return roomPtr;
}

bool RoomManager::ActivateRoom(const EntityHandle& roomHandle)
{
    bool isSuccess = false;
    if (Room* roomInstance = GetRoomPtr(roomHandle))
    {
        // pending deletion?
        if (cxx::contains(mRemoveQueue, roomHandle))
        {
            gConsole.LogMessage(eLogLevel_Warning, "Attempting to activate deleted room instance");
        }
        // already active?
        else if (cxx::contains(mActiveRooms, roomInstance) || cxx::contains(mRegistrationQueue, roomHandle))
        {
            gConsole.LogMessage(eLogLevel_Warning, "Attempting to activate already activated room instance");
        }
        else // success
        {
            // register later
            mRegistrationQueue.push_back(roomHandle);
            // handle spawn immediately
            roomInstance->SpawnInstance();
            isSuccess = true;
        }
    }
    else
    {
        gConsole.LogMessage(eLogLevel_Warning, "Attempting to activate non exising room instance");
    }
    cxx_assert(isSuccess);
    return isSuccess;
}

bool RoomManager::ActivateRoom(EntityUid instanceUid)
{
    EntityHandle roomHandle = FindRoom(instanceUid);
    return ActivateRoom(roomHandle);
}

bool RoomManager::DeleteRoom(const EntityHandle& roomHandle)
{
    bool isSuccess = false;

    if (cxx::contains(mRemoveQueue, roomHandle))
    {
        isSuccess = true;
    }
    else
    {
        // make sure to discard pending degistration
        cxx::erase(mRegistrationQueue, roomHandle);

        if (Room* roomInstance = GetRoomPtr(roomHandle))
        {
            roomInstance->MarkDeleted();

            mRemoveQueue.push_back(roomHandle);
            isSuccess = true;
        }
        else
        {
            gConsole.LogMessage(eLogLevel_Warning, "Attempting to delete non exising room instance");
        }
    }
    cxx_assert(isSuccess);
    return isSuccess;
}

bool RoomManager::DeleteRoom(EntityUid instanceUid)
{
    EntityHandle roomHandle = FindRoom(instanceUid);
    return DeleteRoom(roomHandle);
}

bool RoomManager::ExistsOnMap(const EntityHandle& roomHandle) const
{
    if (Room* roomInstance = GetRoomPtr(roomHandle))
    {
        return roomInstance->ExistsOnMap();
    }
    return false;
}

cxx::uniqueptr<Room> RoomManager::NewRoomInstance() const
{
    static SimplePool<Room> roomsPool = (
        [](Room* object)
        {
            object->DespawnInstance();
            object->OnRecycle();
        });

    Room* roomInstancePtr = roomsPool.Acquire();
    return std::move(cxx::uniqueptr<Room> (roomInstancePtr, [](Room* object)
    {
        if (object)
        {
            roomsPool.Return(object);
        }
    }));
}

template<typename TRoomController>
cxx::uniqueptr<RoomController> RoomManager::NewControllerInstance() const
{
    static SimplePool<TRoomController> controllersPool = {
        [](TRoomController* object)
        {
            object->DespawnInstance();
            object->OnRecycle();
        }};

    RoomController* controllerInstance = controllersPool.Acquire();
    cxx_assert(controllerInstance);

    return std::move(cxx::uniqueptr<RoomController> (controllerInstance, [](RoomController* object)
    {
        if (object)
        {
            TRoomController* speciticControllerType = static_cast<TRoomController*>(object);
            controllersPool.Return(speciticControllerType);
        }
    }));
}

cxx::uniqueptr<RoomController> RoomManager::NewControllerInstance(RoomDefinition* roomDefinition) const
{
    cxx_assert(roomDefinition);
    switch (roomDefinition->mRoomType)
    {
        case RoomTypeId_DungeonHeart        : return NewControllerInstance<DungeonHeartRoomController>();
        case RoomTypeId_Temple              : return NewControllerInstance<TempleRoomController>();
        case RoomTypeId_HeroGate_Frontend   : return NewControllerInstance<HeroGateFrontendRoomController>();
        case RoomTypeId_WorkShop            : return NewControllerInstance<WorkshopRoomController>();
        case RoomTypeId_Library             : return NewControllerInstance<LibraryRoomController>();
        case RoomTypeId_TortureChamber      : return NewControllerInstance<TortureChamberRoomController>();
        case RoomTypeId_TrainingRoom        : return NewControllerInstance<TrainingRoomController>();
        case RoomTypeId_Hatchery            : return NewControllerInstance<HatcheryRoomController>();
        case RoomTypeId_Treasury            : return NewControllerInstance<TreasuryRoomController>();
    }
    // default
    return NewControllerInstance<RoomController>();
}

void RoomManager::ProcessRegistrationQueue()
{
    while (!mRegistrationQueue.empty())
    {
        const EntityHandle roomHandle = mRegistrationQueue.back();

        // get slot
        if (roomHandle.IsRoom() && (roomHandle.mIndex < mRoomSlots.size()))
        {
            RoomInstanceSlot& roomSlot = mRoomSlots[roomHandle.mIndex];
            if (roomHandle.mGeneration == roomSlot.mGeneration)
            {
                Room* roomInstance = roomSlot.mInstance.get();
                RegisterRoom(roomInstance);
            }
            else
            {
                cxx_assert(false);
            }
        }
        mRegistrationQueue.pop_back();
    }
}

void RoomManager::ProcessRemoveQueue()
{
    while (!mRemoveQueue.empty())
    {
        const EntityHandle roomHandle = mRemoveQueue.back();
        // get slot
        if (roomHandle.IsRoom() && (roomHandle.mIndex < mRoomSlots.size()))
        {
            RoomInstanceSlot& roomSlot = mRoomSlots[roomHandle.mIndex];
            if (roomHandle.mGeneration == roomSlot.mGeneration)
            {
                Room* roomInstance = roomSlot.mInstance.get();
                roomInstance->DespawnInstance();
                UnregisterRoom(roomInstance);
                // update slot
                ++roomSlot.mGeneration;
                cxx_assert(roomSlot.mGeneration != 0);
                // release object
                roomSlot.mInstance.reset();
                roomSlot.mController.reset();
            }
            else
            {
                cxx_assert(false);
            }
        }
        mRemoveQueue.pop_back();
    }
}

void RoomManager::DestroyRooms()
{
    mRoomSlots.clear();
    mInstanceUidsMap.clear();
    mActiveRooms.clear();
    mRemoveQueue.clear();
    mActiveRoomsByType.clear();
    mRegistrationQueue.clear();
}

void RoomManager::ConfigureNewRoomInstance(Room* roomInstance, 
    RoomController* controller, 
    RoomDefinition* definition, 
    EntityUid instanceUid, eDirection roomDirection, ePlayerID ownerId)
{
    cxx_assert(roomInstance);

    // setup components
    if ((definition->mRoomType == RoomTypeId_DungeonHeart) ||
        (definition->mRoomType == RoomTypeId_Treasury))
    {
        roomInstance->AddComponent<MoneyComponent>();
    }

    TileConstructionSet& constructionSet = gGameWorld.GetTileConstructionSet();

    // assing room constructor
    RoomTileConstructor* roomConstructor = constructionSet.GetRoomConstructor(definition);
    roomInstance->ConfigureInstance(instanceUid, controller, roomConstructor, definition, roomDirection, ownerId);
}

void RoomManager::RegisterRoom(Room* roomInstance)
{
    cxx_assert(roomInstance);

    mActiveRooms.push_back(roomInstance); // it's safe to add even during update

    // update maps
    const RoomTypeId roomTypeId = roomInstance->GetTypeId();
    mActiveRoomsByType[roomTypeId].push_back(roomInstance);
}

void RoomManager::UnregisterRoom(Room* roomInstance)
{
    cxx_assert(roomInstance);

    // cleanup lists
    cxx::erase(mActiveRooms, roomInstance);

    // update maps
    mInstanceUidsMap.erase(roomInstance->GetInstanceUid());

    const RoomTypeId roomTypeId = roomInstance->GetTypeId();
    cxx::erase(mActiveRoomsByType[roomTypeId], roomInstance);
}
