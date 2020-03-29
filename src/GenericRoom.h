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
    Rect2D mOccupationArea; // approximate size in tiles
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

    // force construct geometries of all room tiles and wall sections
    void BuildTilesMesh();

    // construct geometries of all invalidated room tiles and wall sections
    void UpdateTilesMesh();

protected:
    void IncludeTiles(const TilesArray& mapTiles);

    void ReevaluateOccupationArea();
    void ReevaluateInnerSquares();
    void ReevaluateWallSections();
    
    void ReleaseWallSections();
    void ScanWallSection(MapTile* mapTile, WallSection* section) const;
    void FinalizeWallSection(WallSection* section);

    // overridables

    // each time room gets modified it must be reconfigured
    // for example, tiles added or removed, wall sections updated
    virtual void OnReconfigure() {}

protected:
    // walls construction
    void ConstructWalls(DungeonBuilder& builder, bool forceConstructAll);

    // construct floor tiles mesh
    void ConstructFloorTiles(DungeonBuilder& builder, const TilesArray& mapTiles);

    // tile mesh construction procs
    void ConstructTiles_DoubleQuad      (DungeonBuilder& builder, const TilesArray& mapTiles);
    void ConstructTiles_5x5Rotated      (DungeonBuilder& builder, const TilesArray& mapTiles);
    void ConstructTiles_3x3             (DungeonBuilder& builder, const TilesArray& mapTiles);
    void ConstructTiles_Quad            (DungeonBuilder& builder, const TilesArray& mapTiles);
    void ConstructTiles_Normal          (DungeonBuilder& builder, const TilesArray& mapTiles);
    void ConstructTiles_HeroGateFrontEnd(DungeonBuilder& builder, const TilesArray& mapTiles);
    void ConstructTiles_HeroGate3x1     (DungeonBuilder& builder, const TilesArray& mapTiles);

protected:
    std::vector<WallSection*> mWallSections;
    TilesArray mInnerTiles;
};