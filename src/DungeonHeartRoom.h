#pragma once

#include "GenericRoom.h"

// dungeon heart room logic
class DungeonHeartRoom: public GenericRoom
{
public:
    DungeonHeartRoom(RoomDefinition* definition, ePlayerID owner, RoomInstanceID uid);
    ~DungeonHeartRoom();

private:
    // override GenericRoom
    void Reconfigure() override;
};
