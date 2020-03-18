#pragma once

using TerrainTypeID = unsigned int; // for sake of flexibility, do not rely on specific id
const TerrainTypeID TerrainType_Null = 0; // invalid identifier

using RoomTypeID = unsigned int; // for sake of flexibility, do not rely on specific id
const RoomTypeID RoomTypeID_Null = 0; // invalid identifier

using GameObjectClassID = unsigned int; // for sake of flexibility, do not rely on specific id
const GameObjectClassID GameObjectClassID_Null = 0; // invalid identifier

using CreatureClassID = unsigned int;
const CreatureClassID CreatureClassID_Null = 0; // invalid identifier

// possible player identifiers
enum ePlayerID
{
    ePlayerID_Null, // not a player
    ePlayerID_Good,
    ePlayerID_Neutral,
    ePlayerID_Keeper1,
    ePlayerID_Keeper2,
    ePlayerID_Keeper3,
    ePlayerID_Keeper4,
    ePlayerID_COUNT
};

decl_enum_strings(ePlayerID);

// player type
enum ePlayerType
{
    ePlayerType_Dummy,
    ePlayerType_Human,
    ePlayerType_Computer,
    ePlayerType_COUNT
};

decl_enum_strings(ePlayerType);

// computer AI type
enum eComputerAI 
{
    eComputerAI_MasterKeeper,
    eComputerAI_Conqueror,
    eComputerAI_Psychotic,
    eComputerAI_Stalwart,
    eComputerAI_Greyman,
    eComputerAI_Idiot,
    eComputerAI_Guardian,
    eComputerAI_ThickSkinned,
    eComputerAI_Paranoid,
    ePlayerAIType_COUNT
};

decl_enum_strings(eComputerAI);

// room tile construction type
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
    eRoomTileConstruction_HeroGateFrontEnd,
    eRoomTileConstruction_HeroGate_3_by_1,
    eRoomTileConstruction_COUNT,
};

decl_enum_strings(eRoomTileConstruction);

// Object material identifier
enum eGameObjectMaterial
{
    eGameObjectMaterial_None,
    eGameObjectMaterial_Flesh,
    eGameObjectMaterial_Rock,
    eGameObjectMaterial_Wood,
    eGameObjectMaterial_Metal1,
    eGameObjectMaterial_Metal2,
    eGameObjectMaterial_Magic,
    eGameObjectMaterial_Glass,
    eGameObjectMaterial_COUNT
};

decl_enum_strings(eGameObjectMaterial);