#pragma once

#define DUNGEON_CELL_HALF_SIZE  0.5f
#define DUNGEON_CELL_SIZE       1.0f
#define TERRAIN_FLOOR_LEVEL     1.0f
#define TERRAIN_BLOCK_HEIGHT    1.0f

// forwards
class GameMap;
class MapTile;
class GameObject;
class GameObjectComponent;
class AnimatingModelComponent;
class TerrainMeshComponent;
class StaticMeshComponent;
class WaterLavaMeshComponent;
class TransformComponent;

// terrain type identifier
enum TerrainTypeID: unsigned int // for sake of flexibility, do not rely on specific id
{
    TerrainType_Null = 0 // invalid identifier
};

// room type identifier
enum RoomTypeID: unsigned int // for sake of flexibility, do not rely on specific id
{
    RoomType_Null = 0 // invalid identifier
};

// game object type identifier
enum GameObjectTypeID: unsigned int // for sake of flexibility, do not rely on specific id
{
    GameObjectType_Null = 0 // invalid identifier
};

// create type identifier
enum CreatureTypeID: unsigned int
{
    CreatureType_Null = 0 // invalid identifier
};

// weak pointer to game object instance
using GameObjectHandle = cxx::handle<GameObject>;

// game object instance unique identifier
using GameObjectInstanceID = unsigned long long;

// scene object component type
enum eGameObjectComponent
{
    eGameObjectComponent_Transform,
    eGameObjectComponent_TerrainMesh,
    eGameObjectComponent_WaterLavaMesh,
    eGameObjectComponent_AnimatingModel,
    eGameObjectComponent_StaticMesh,

    eGameObjectComponent_Count,
};

decl_enum_strings(eGameObjectComponent);

// array of map tiles
using TilesArray = std::vector<MapTile*>;

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
    eComputerAI_COUNT
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

// block face identifier
enum eTileFace 
{
    eTileFace_SideN, // wall facing North, only specified for solid tiles
    eTileFace_SideE, // wall facing East,  only specified for solid tiles
    eTileFace_SideS, // wall facing South, only specified for solid tiles
    eTileFace_SideW, // wall facing Worth, only specified for solid tiles
    eTileFace_Floor, // bottom, does not specified for solid tiles
    eTileFace_Ceiling, // only specified for solid tiles or bridges
    eTileFace_COUNT
};

decl_enum_strings(eTileFace);

// direction

//
//   NW   N   NE
//      \ | /
//  W <--- ---> E
//      / | \ 
//   SW   S   SE
//

enum eDirection 
{
    eDirection_N,    // North
    eDirection_NE,
    eDirection_E,    // East
    eDirection_SE,
    eDirection_S,    // South
    eDirection_SW,
    eDirection_W,    // West
    eDirection_NW,
    eDirection_COUNT
};

decl_enum_strings(eDirection);

static_assert(eDirection_COUNT == 8, "NUM_TERRAIN_DIRECTIONS");

namespace
{
    // directions in clockwise order
    static const eDirection gDirectionsCW[] 
    {
        eDirection_N, eDirection_NE, eDirection_E, eDirection_SE,
        eDirection_S, eDirection_SW, eDirection_W, eDirection_NW,
    };

    // directions in counter clockwise order
    static const eDirection gDirectionsCCW[] 
    {
        eDirection_NW, eDirection_W, eDirection_SW, eDirection_S,
        eDirection_SE, eDirection_E, eDirection_NE, eDirection_N,
    };

    // straight directions in clockwise order
    static const eDirection gStraightDirections[] 
    {
        eDirection_N, eDirection_E, eDirection_S, eDirection_W
    };
}

// get world direction of block face
// @param faceid: Block face identifier
inline eDirection FaceIdToDirection(eTileFace faceid)
{
    switch (faceid)
    {
        case eTileFace_SideN: return eDirection_N;
        case eTileFace_SideE: return eDirection_E;
        case eTileFace_SideS: return eDirection_S;
        case eTileFace_SideW: return eDirection_W;
    }
    debug_assert(false);
    return eDirection_N;
}

// get block face in direction
// @param direction: Source direction
inline eTileFace DirectionToFaceId(eDirection direction)
{
    switch (direction)
    {
        case eDirection_N: return eTileFace_SideN;
        case eDirection_E: return eTileFace_SideE;
        case eDirection_S: return eTileFace_SideS;
        case eDirection_W: return eTileFace_SideW;
    }
    debug_assert(false);
    return eTileFace_SideN;
}

// get opposite direction
// @param direction: Source direction
inline eDirection GetOppositeDirection(eDirection direction) 
{
    switch (direction)
    {
        case eDirection_N : return eDirection_S;
        case eDirection_NE: return eDirection_SW;
        case eDirection_E : return eDirection_W;
        case eDirection_SE: return eDirection_NW;
        case eDirection_S : return eDirection_N;
        case eDirection_SW: return eDirection_NE;
        case eDirection_W : return eDirection_E;
        case eDirection_NW: return eDirection_SE;
    }
    debug_assert(false);
    return direction;
}

// get next direction clockwise
inline eDirection GetNextDirectionCW(eDirection direction)
{
    switch (direction)
    {
        case eDirection_N : return eDirection_NE;
        case eDirection_NE: return eDirection_E;
        case eDirection_E : return eDirection_SE;
        case eDirection_SE: return eDirection_S;
        case eDirection_S : return eDirection_SW;
        case eDirection_SW: return eDirection_W;
        case eDirection_W : return eDirection_NW;
        case eDirection_NW: return eDirection_N;
    }
    debug_assert(false);
    return direction;
}

// get next direction counterclockwise
inline eDirection GetNextDirectionCCW(eDirection direction)
{
    switch (direction)
    {
        case eDirection_N : return eDirection_NW;
        case eDirection_NE: return eDirection_N;
        case eDirection_E : return eDirection_NE;
        case eDirection_SE: return eDirection_E;
        case eDirection_S : return eDirection_SE;
        case eDirection_SW: return eDirection_S;
        case eDirection_W : return eDirection_SW;
        case eDirection_NW: return eDirection_W;
    }
    debug_assert(false);
    return direction;
}

// get vector for direction
inline Point GetDirectionVector(eDirection direction)
{
    Point point_vector (0, 0);
    if (direction == eDirection_N || direction == eDirection_NE || direction == eDirection_NW)
    {
        point_vector.y = -1;
    }
    if (direction == eDirection_S || direction == eDirection_SE || direction == eDirection_SW)
    {
        point_vector.y = 1;
    }
    if (direction == eDirection_E || direction == eDirection_NE || direction == eDirection_SE)
    {
        point_vector.x = 1;
    }
    if (direction == eDirection_W || direction == eDirection_SW || direction == eDirection_NW)
    {
        point_vector.x = -1;
    }
    debug_assert(point_vector.x || point_vector.y);
    return point_vector; 
}

// test whether direction is one of NESW
inline bool IsStraightDirection(eDirection direction) 
{
    return direction == eDirection_N || direction == eDirection_E || direction == eDirection_S || direction == eDirection_W;
}

// test whether direction is one of NE,SE,SW,NW
inline bool IsDiagonalDirection(eDirection direction)
{
    return direction == eDirection_NE || direction == eDirection_SE || direction == eDirection_SW || direction == eDirection_NW;
}

// compute game map block bounding box
// @param blockLocation: Logical position
// @param outputBounds: Output bounds
inline void GetMapBlockBounds(const Point& blockLocation, cxx::aabbox& outputBounds)
{
    outputBounds.clear();
    // min
    outputBounds.mMin.x = (blockLocation.x * DUNGEON_CELL_SIZE) - DUNGEON_CELL_HALF_SIZE;
    outputBounds.mMin.y = 0.0f;
    outputBounds.mMin.z = (blockLocation.y * DUNGEON_CELL_SIZE) - DUNGEON_CELL_HALF_SIZE;
    // max
    outputBounds.mMax.x = outputBounds.mMin.x + DUNGEON_CELL_SIZE;
    outputBounds.mMax.y = 3.0f;
    outputBounds.mMax.z = outputBounds.mMin.z + DUNGEON_CELL_SIZE;
}

// compute game map block center in world coordinates
inline void GetMapBlockCenter(const Point& blockLocation, glm::vec3& outputCoord)
{
    outputCoord.x = blockLocation.x * DUNGEON_CELL_SIZE;
    outputCoord.y = 1.0f;
    outputCoord.z = blockLocation.y * DUNGEON_CELL_SIZE;
}

enum eMapInteractionMode
{
    eMapInteractionMode_Free, // can tag terrain, pick creatures, interact with objects
    eMapInteractionMode_CastSpells, // can cast spells
    eMapInteractionMode_ConstructRooms, // can tag terrain, construct rooms
    eMapInteractionMode_SellRooms, // can sell rooms
    eMapInteractionMode_ConstructTraps, // can place traps
    eMapInteractionMode_DigTerrain, // can claim or destroy terrain tiles
};
decl_enum_strings(eMapInteractionMode);