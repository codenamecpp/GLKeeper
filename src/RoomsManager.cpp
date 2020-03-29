#include "pch.h"
#include "RoomsManager.h"
#include "GenericRoom.h"
#include "TempleRoom.h"

RoomsManager gRoomsManager;

bool RoomsManager::Initialize()
{
    mRoomIDsCounter = 0;
    return true;
}

void RoomsManager::Deinit()
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
            currentRoom->mOwnerIdentifier == owner)
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
    return ++mRoomIDsCounter; // todo
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

GenericRoom* RoomsManager::CreateRoomInstance(RoomDefinition* definition, ePlayerID owner, const TilesArray& roomTiles)
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

    if (genericRoom == nullptr && definition->mRoomName == ROOM_NAME_TEMPLE)
    {
        genericRoom = new TempleRoom(definition, owner, uid);
    }

    if (genericRoom == nullptr) // create generic room
    {
        genericRoom = new GenericRoom(definition, owner, uid);
    }

    mRoomsList.push_back(genericRoom);
    return genericRoom;
}
