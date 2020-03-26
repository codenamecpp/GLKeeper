#include "pch.h"
#include "GenericRoom.h"
#include "MapTile.h"

GenericRoom::GenericRoom(RoomDefinition* definition, RoomInstanceID uid)
    : mDefinition(definition)
    , mInstanceID(uid)
    , mOwnerIdentifier(ePlayerID_Null)
{
    debug_assert(mDefinition);
}

GenericRoom::~GenericRoom()
{
}

void GenericRoom::UpdateFrame()
{

}

