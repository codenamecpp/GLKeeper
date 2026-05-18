#pragma once

#include "RoomTileConstructor.h"

class RoomTileConstructor_Normal: public RoomTileConstructor
{
public:
    RoomTileConstructor_Normal(TileConstructor& baseConstructor);
    // override RoomTileConstructor
    void ConstructRoomFloor(Room* roomInstance, cxx::span<MapTile*> floorTiles) override;
};