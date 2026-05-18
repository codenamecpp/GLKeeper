#pragma once

#include "RoomTileConstructor.h"

class RoomTileConstructor_Quad: public RoomTileConstructor
{
public:
    RoomTileConstructor_Quad(TileConstructor& baseConstructor);
    // override RoomTileConstructor
    void ConstructRoomFloor(Room* roomInstance, cxx::span<MapTile*> floorTiles) override;
};