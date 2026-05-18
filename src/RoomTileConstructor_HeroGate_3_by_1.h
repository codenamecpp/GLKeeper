#pragma once

//////////////////////////////////////////////////////////////////////////

#include "RoomTileConstructor.h"

//////////////////////////////////////////////////////////////////////////

class RoomTileConstructor_HeroGate_3_by_1: public RoomTileConstructor
{
public:
    RoomTileConstructor_HeroGate_3_by_1(TileConstructor& baseConstructor);
    // override RoomTileConstructor
    void ConstructRoomFloor(Room* roomInstance, cxx::span<MapTile*> floorTiles) override;
    void ConstructRoomWalls(Room* roomInstance, cxx::span<MapTile*> wallTiles, eTileFace wallFace) override;
};

//////////////////////////////////////////////////////////////////////////
