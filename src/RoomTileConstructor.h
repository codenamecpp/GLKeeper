#pragma once

//////////////////////////////////////////////////////////////////////////

class TileConstructor;

//////////////////////////////////////////////////////////////////////////

class RoomTileConstructor: public cxx::noncopyable
{
public:
    RoomTileConstructor(TileConstructor& baseConstructor);
    virtual ~RoomTileConstructor();
    virtual void ConstructRoomFloor(Room* roomInstance, cxx::span<MapTile*> floorTiles);
    virtual void ConstructRoomWalls(Room* roomInstance, cxx::span<MapTile*> wallTiles, eTileFace wallFace);
protected:
    TileConstructor& mBaseTileConstructor;
};