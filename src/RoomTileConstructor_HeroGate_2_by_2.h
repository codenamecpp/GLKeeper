#pragma once

//////////////////////////////////////////////////////////////////////////

#include "RoomTileConstructor.h"

//////////////////////////////////////////////////////////////////////////

class RoomTileConstructor_HeroGate_2_by_2: public RoomTileConstructor
{
public:
    RoomTileConstructor_HeroGate_2_by_2(TileConstructor& baseConstructor);
    // override RoomTileConstructor
    void ConstructRoomFloor(Room* roomInstance, cxx::span<MapTile*> floorTiles) override;
};

//////////////////////////////////////////////////////////////////////////