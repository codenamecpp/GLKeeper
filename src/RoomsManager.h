#pragma once

#include "RoomsDefs.h"
#include "ScenarioDefs.h"

class RoomsManager: public cxx::noncopyable
{
public:
    // readonly
    std::vector<GenericRoom*> mRoomsList;

public:

    // setup rooms manager internal resources
    bool Initialize();
    void Deinit();

    // process single frame logic
    void UpdateFrame();

    // find room instance by unique identifier
    // @param uid: Unique identifier
    GenericRoom* GetRoomInstance(RoomInstanceID uid) const;
    
    // find first room instance of specific type
    GenericRoom* GetRoomInstance(RoomDefinition* definition) const;
    GenericRoom* GetRoomInstance(RoomTypeID typeIdentifier) const;
    // @param owner: Owner player identifier
    GenericRoom* GetRoomInstance(RoomDefinition* definition, ePlayerID owner) const;
    GenericRoom* GetRoomInstance(RoomTypeID typeIdentifier, ePlayerID owner) const;

private:
    void DestroyRoomsList();

private:
};

extern RoomsManager gRoomsManager;