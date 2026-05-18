#pragma once

#include "RoomTileConstructor.h"

class RoomTileConstructor_DoubleQuad: public RoomTileConstructor
{
public:
    RoomTileConstructor_DoubleQuad(TileConstructor& baseConstructor);
    // override RoomTileConstructor
    void ConstructRoomFloor(Room* roomInstance, cxx::span<MapTile*> floorTiles) override;
};