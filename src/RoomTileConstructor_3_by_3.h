#pragma once

#include "RoomTileConstructor.h"

class RoomTileConstructor_3_by_3: public RoomTileConstructor
{
public:
    RoomTileConstructor_3_by_3(TileConstructor& baseConstructor);
    // override RoomTileConstructor
    void ConstructRoomFloor(Room* roomInstance, cxx::span<MapTile*> floorTiles) override;
};