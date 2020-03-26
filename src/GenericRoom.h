#pragma once

#include "ScenarioDefs.h"
#include "GameDefs.h"
#include "RoomsDefs.h"

// generic room class
class GenericRoom: public cxx::handled_object<GenericRoom>
{
    friend class RoomsManager;

public:
    // readonly
    RoomDefinition* mDefinition; // cannot be null
    RoomInstanceID mInstanceID; // instance unique identifier
    TilesArray mRoomTiles;
    Rect2D mRoomTilesArea; // approximate size in tiles
    ePlayerID mOwnerIdentifier;

public:
    GenericRoom(RoomDefinition* definition, RoomInstanceID uid);
    virtual ~GenericRoom();

    // process single frame logic
    void UpdateFrame();

protected:


    TilesArray mInnerTiles;

};