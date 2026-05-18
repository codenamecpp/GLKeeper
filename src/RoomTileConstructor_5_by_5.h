#pragma once

#include "RoomTileConstructor.h"

class RoomTileConstructor_5_by_5: public RoomTileConstructor
{
public:
    RoomTileConstructor_5_by_5(TileConstructor& baseConstructor);
    // override RoomTileConstructor
    void ConstructRoomFloor(Room* roomInstance, cxx::span<MapTile*> floorTiles) override;
};