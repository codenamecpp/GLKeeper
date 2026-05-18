#pragma once

#include "GameMapDefs.h"
#include "EntityDefs.h"

//////////////////////////////////////////////////////////////////////////

// forward declarations
class Room;
class RoomController;
class RoomManager;

//////////////////////////////////////////////////////////////////////////

using RoomTypeId = unsigned int;
// well known room type ids
enum : RoomTypeId
{ 
    RoomTypeId_Null = 0,
    RoomTypeId_Treasury = 1,
    RoomTypeId_Lair = 2,
    RoomTypeId_Portal = 3,
    RoomTypeId_Hatchery = 4,
    RoomTypeId_DungeonHeart = 5,
    RoomTypeId_Library = 6,
    RoomTypeId_TrainingRoom = 7,
    RoomTypeId_WoodenBridge = 8,
    RoomTypeId_GuardRoom = 9,
    RoomTypeId_WorkShop = 10,
    RoomTypeId_Prison = 11,
    RoomTypeId_TortureChamber = 12,
    RoomTypeId_Temple = 13,
    RoomTypeId_Graveyard = 14,
    RoomTypeId_Casino = 15,
    RoomTypeId_CombatPit = 16,
    RoomTypeId_StoneBridge = 17,
    RoomTypeId_HeroGate_Final = 18,
    RoomTypeId_HeroGate_Tile = 19,
    RoomTypeId_HeroGate_2x2 = 20,
    RoomTypeId_HeroGate_Frontend = 21,
    RoomTypeId_HeroStoneBridge = 22,
    RoomTypeId_HeroGate_3x1 = 23,
    RoomTypeId_MercenaryGate = 24,
    RoomTypeId_HeroPortal = 25,
    RoomTypeId_Crypt = 26,
};

//////////////////////////////////////////////////////////////////////////

struct RoomFurnitureSlot
{
public:

    enum ePositioning
    {
        ePositioning_Default,
        ePositioning_SubTileNW,
        ePositioning_SubTileNE,
        ePositioning_SubTileSE,
        ePositioning_SubTileSW,
        ePositioning_WallN,
        ePositioning_WallE, // :)
        ePositioning_WallS,
        ePositioning_WallW,
    };

    enum eFaceRotation
    {
        eFaceRotation_0, // none
        eFaceRotation_90_Neg,
        eFaceRotation_90_Pos,
        eFaceRotation_180,
        eFaceRotation_Random
    };

public:
    // note: 
    // - multiple objects can share a single tile, but each one gets its own slot
    MapPoint2D mTileLocation {};

    eFaceRotation mObjectRotation = eFaceRotation_0; // rotate object arount world Y (UP) axis
    GameObjectClassId mObjectClassId = GameObjectClassId_Null;
    eGameObjectMeshId mObjectMeshId = eGameObjectMeshId_Main;
    ePositioning mObjectPositioning = ePositioning_Default;

    EntityHandle mObjectHandle {}; // will be set after evaluation
};

//////////////////////////////////////////////////////////////////////////

struct RoomStorageSlot
{
public:
    // note: 
    // - multiple objects can share a single tile, but each one gets its own slot
    MapPoint2D mTileLocation {};

    EntityHandle mObjectHandle {};
};

//////////////////////////////////////////////////////////////////////////

// Room tile construction
enum eRoomTileConstruction 
{
    eRoomTileConstruction_Complete,
    eRoomTileConstruction_Quad, // usually bridge
    eRoomTileConstruction_3_by_3, // usually portal room
    eRoomTileConstruction_3_by_3_Rotated,
    eRoomTileConstruction_Normal, // most of the room types for ex. treasury, casino, lair etc
    eRoomTileConstruction_CenterPool,
    eRoomTileConstruction_DoubleQuad, // prison, temple, combat pit
    eRoomTileConstruction_5_by_5_Rotated, // dungeon heart
    eRoomTileConstruction_HeroGate,
    eRoomTileConstruction_HeroGateTile,
    eRoomTileConstruction_HeroGate_2_by_2,
    eRoomTileConstruction_HeroGateFrontend,
    eRoomTileConstruction_HeroGate_3_by_1,
    eRoomTileConstruction_COUNT,
};

//////////////////////////////////////////////////////////////////////////