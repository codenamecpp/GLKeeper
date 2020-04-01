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

    void EnterWorld();
    void ClearWorld();

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

    // create new room instance
    GenericRoom* CreateRoomInstance(RoomDefinition* definition, ePlayerID owner);
    GenericRoom* CreateRoomInstance(RoomDefinition* definition, ePlayerID owner, const TilesArray& roomTiles);

private:
    void DestroyRoomsList();

    RoomInstanceID GenerateNewRoomInstanceID();
    GenericRoom* CreateRoomInstance(RoomDefinition* definition, ePlayerID owner, RoomInstanceID uid);

private:
    RoomInstanceID mRoomIDsCounter = 0;
};

extern RoomsManager gRoomsManager;