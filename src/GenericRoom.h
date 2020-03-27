#pragma once

#include "ScenarioDefs.h"
#include "GameDefs.h"
#include "RoomsDefs.h"

// forwards
class DungeonBuilder;

// generic room class
class GenericRoom: public cxx::handled_object<GenericRoom>
{
    friend class RoomsManager;

public:
    // readonly
    RoomDefinition* mDefinition; // cannot be null
    RoomInstanceID mInstanceID; // instance unique identifier
    TilesArray mRoomTiles;
    Rect2D mLocationArea; // approximate size in tiles
    ePlayerID mOwnerIdentifier;

public:
    GenericRoom(RoomDefinition* definition, ePlayerID owner, RoomInstanceID uid);
    virtual ~GenericRoom();

    // process single frame logic
    void UpdateFrame();

    // remove specific tiles from room, this will lead to reevaluation of remaining tiles
    void ReleaseTiles(const TilesArray& mapTiles);
    void ReleaseTiles();

    // expand room with new tiles
    void EnlargeRoom(const TilesArray& mapTiles);

protected:
    // tile mesh construction
    void ConstructTile_DoubleQuad(DungeonBuilder& builder, const TilesArray& mapTiles);
    void ConstructTile_5x5Rotated(DungeonBuilder& builder, const TilesArray& mapTiles);
    void ConstructTile_3x3(DungeonBuilder& builder, const TilesArray& mapTiles);
    void ConstructTile_Quad(DungeonBuilder& builder, const TilesArray& mapTiles);
    void ConstructTile_Normal(DungeonBuilder& builder, const TilesArray& mapTiles);
    void ConstructTile_HeroGateFrontEnd(DungeonBuilder& builder, const TilesArray& mapTiles);
    void ConstructTile_HeroGate3x1(DungeonBuilder& builder, const TilesArray& mapTiles);

protected:


    TilesArray mInnerTiles;

};