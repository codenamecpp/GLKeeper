#pragma once

#include "RoomTileConstructor_5_by_5.h"

class RoomTileConstructor_DungeonHeart: public RoomTileConstructor_5_by_5
{
public:
    RoomTileConstructor_DungeonHeart(TileConstructor& baseConstructor);
    // override RoomTileConstructor
    void ConstructRoomFloor(Room* roomInstance, cxx::span<MapTile*> floorTiles) override;
};