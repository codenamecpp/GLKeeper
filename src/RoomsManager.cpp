#include "pch.h"
#include "RoomsManager.h"
#include "GenericRoom.h"
#include "TempleRoom.h"
#include "DungeonHeartRoom.h"

RoomsManager gRoomsManager;

bool RoomsManager::Initialize()
{
    return true;
}

void RoomsManager::Deinit()
{   

}

void RoomsManager::EnterWorld()
{
    mRoomIDsCounter = 0;
}

void RoomsManager::ClearWorld()
{
    DestroyRoomsList();
}

void RoomsManager::UpdateFrame()
{

}

GenericRoom* RoomsManager::GetRoomInstance(RoomInstanceID uid) const
{
    for (GenericRoom* currentRoom: mRoomsList)
    {
        if (currentRoom->mInstanceID == uid)
            return currentRoom;
    }
    return nullptr;
}

GenericRoom* RoomsManager::GetRoomInstance(RoomDefinition* definition) const
{
    debug_assert(definition);
    if (definition)
    {
        return GetRoomInstance(definition->mRoomType);
    }
    return nullptr;
}

GenericRoom* RoomsManager::GetRoomInstance(RoomTypeID typeIdentifier) const
{
    for (GenericRoom* currentRoom: mRoomsList)
    {
        if (currentRoom->mDefinition->mRoomType == typeIdentifier)
            return currentRoom;
    }
    return nullptr;
}

GenericRoom* RoomsManager::GetRoomInstance(RoomDefinition* definition, ePlayerID owner) const
{
    debug_assert(definition);
    if (definition)
    {
        return GetRoomInstance(definition->mRoomType, owner);
    }
    return nullptr;
}

GenericRoom* RoomsManager::GetRoomInstance(RoomTypeID typeIdentifier, ePlayerID owner) const
{
    for (GenericRoom* currentRoom: mRoomsList)
    {
        if (currentRoom->mDefinition->mRoomType == typeIdentifier && 
            currentRoom->mOwnerID == owner)
        {
            return currentRoom;
        }
    }
    return nullptr;
}

void RoomsManager::DestroyRoomsList()
{
    for (GenericRoom* currentRoom: mRoomsList)
    {
        SafeDelete(currentRoom);
    }
    mRoomsList.clear();
}

RoomInstanceID RoomsManager::GenerateNewRoomInstanceID()
{
    return ++mRoomIDsCounter; // todo: need better way to generate id
}

GenericRoom* RoomsManager::CreateRoomInstance(RoomDefinition* definition, ePlayerID owner)
{
    debug_assert(definition);
    if (definition == nullptr)
        return nullptr;

    RoomInstanceID uid = GenerateNewRoomInstanceID();

    GenericRoom* genericRoom = CreateRoomInstance(definition, owner, uid);
    return genericRoom;
}

GenericRoom* RoomsManager::CreateRoomInstance(RoomDefinition* definition, ePlayerID owner, const TilesList& roomTiles)
{
    debug_assert(definition);
    if (definition == nullptr)
        return nullptr;

    RoomInstanceID uid = GenerateNewRoomInstanceID();

    GenericRoom* genericRoom = CreateRoomInstance(definition, owner, uid);

    // setup room
    genericRoom->EnlargeRoom(roomTiles);
    return genericRoom;
}

GenericRoom* RoomsManager::CreateRoomInstance(RoomDefinition* definition, ePlayerID owner, RoomInstanceID uid)
{
    GenericRoom* genericRoom = nullptr;

    do
    {
        if (definition->mRoomName == ROOM_NAME_TEMPLE)
        {
            genericRoom = new TempleRoom(definition, owner, uid);
            break;
        }

        if (definition->mRoomName == ROOM_NAME_DUNGEON_HEART)
        {
            genericRoom = new DungeonHeartRoom(definition, owner, uid);
            break;
        }

        // create generic room
        genericRoom = new GenericRoom(definition, owner, uid);

    } while (false);

    debug_assert(genericRoom);

    mRoomsList.push_back(genericRoom);
    return genericRoom;
}

void RoomsManager::DestroyRoomInstance(GenericRoom* roomInstance)
{
    cxx::erase_elements(mRoomsList, roomInstance);

    SafeDelete(roomInstance);
}
