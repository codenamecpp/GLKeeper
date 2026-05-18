#pragma once

//////////////////////////////////////////////////////////////////////////

class GameMap;
class MapTile;

//////////////////////////////////////////////////////////////////////////

#define MAP_TILE_SIZE       1.0f
#define MAP_TILE_HALF_SIZE  (MAP_TILE_SIZE * 0.5f)
#define MAP_FLOOR_LEVEL     1.0f
#define MAP_BLOCK_HEIGHT    1.0f

//////////////////////////////////////////////////////////////////////////

// Cardinal direction

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

namespace
{
    // directions in clockwise order
    static const eDirection gDirectionsCW[] {
        eDirection_N, eDirection_NE, eDirection_E, eDirection_SE,
        eDirection_S, eDirection_SW, eDirection_W, eDirection_NW,
    };

    // directions in counter clockwise order
    static const eDirection gDirectionsCCW[] {
        eDirection_NW, eDirection_W, eDirection_SW, eDirection_S,
        eDirection_SE, eDirection_E, eDirection_NE, eDirection_N,
    };

    // straight directions in clockwise order
    static const eDirection gStraightDirections[] { eDirection_N, eDirection_E, eDirection_S, eDirection_W };
    static const eDirection gDiagonalDirections[] { eDirection_NE, eDirection_SE, eDirection_SW, eDirection_NW };
}

static_assert(eDirection_COUNT == 8, "NUM_TERRAIN_DIRECTIONS");
    
// Get opposite direction
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
    cxx_assert(false);
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
    cxx_assert(false);
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
    cxx_assert(false);
    return direction;
}

inline bool IsStraightDirection(eDirection direction) 
{
    return (direction == eDirection_N) || (direction == eDirection_E) || (direction == eDirection_S) || (direction == eDirection_W);
}

// test whether direction is one of NE,SE,SW,NW
inline bool IsDiagonalDirection(eDirection direction)
{
    return (direction == eDirection_NE) || (direction == eDirection_SE) || (direction == eDirection_SW) || (direction == eDirection_NW);
}

//////////////////////////////////////////////////////////////////////////

// Index of dungeon map tile face
enum eTileFace 
{
    eTileFace_SideN, // wall N, only specified for solid tiles
    eTileFace_SideE, // wall E, only specified for solid tiles
    eTileFace_SideS, // wall S, only specified for solid tiles
    eTileFace_SideW, // wall W, only specified for solid tiles
    eTileFace_Floor, // bottom, does not specified for solid tiles
    eTileFace_Ceiling, // only specified for solid tiles or bridges
    eTileFace_COUNT
};

namespace
{
    static const eTileFace gTileFaces[] 
    {
        eTileFace_SideN, 
        eTileFace_SideE, 
        eTileFace_SideS,
        eTileFace_SideW, 
        eTileFace_Floor, 
        eTileFace_Ceiling,
    };
}

// get world direction of block face
// @param faceid: Block face identifier
inline eDirection TileFaceToDirection(eTileFace faceid)
{
    switch (faceid)
    {
        case eTileFace_SideN: return eDirection_N;
        case eTileFace_SideE: return eDirection_E;
        case eTileFace_SideS: return eDirection_S;
        case eTileFace_SideW: return eDirection_W;
    }
    cxx_assert(false);
    return eDirection_N;
}

// get block face in direction
// @param direction: Source direction
inline eTileFace DirectionToTileFace(eDirection direction)
{
    switch (direction)
    {
        case eDirection_N: return eTileFace_SideN;
        case eDirection_E: return eTileFace_SideE;
        case eDirection_S: return eTileFace_SideS;
        case eDirection_W: return eTileFace_SideW;
    }
    cxx_assert(false);
    return eTileFace_SideN;
}

//////////////////////////////////////////////////////////////////////////

// logical coord or size on game map 2d tiles grid

using MapPoint2D = glm::ivec2;

//////////////////////////////////////////////////////////////////////////

// logical area of game map 2d tiles grid

struct MapArea2D 
{
public:
    MapArea2D() = default; // not initialized by default
    MapArea2D(int coordx, int coordy, int sizex, int sizey)
        : x(coordx)
        , y(coordy)
        , w(sizex)
        , h(sizey)
    {
    }
    MapArea2D(const MapPoint2D& pos)
        : x(pos.x)
        , y(pos.y)
        , w(1)
        , h(1)
    {
    }
    inline bool operator == (const MapArea2D& rhs) const { return (x == rhs.x) && (y == rhs.y) && (w == rhs.w) && (h == rhs.h); }
    inline bool operator != (const MapArea2D& rhs) const { return !(*this == rhs); }
public:
    int x, y;
    int w, h;
};

//////////////////////////////////////////////////////////////////////////

inline bool operator < (const MapPoint2D& lhs, const MapPoint2D& rhs)
{
    return (lhs.y != rhs.y) ? (lhs.y < rhs.y) : (lhs.x < rhs.x);
}

//////////////////////////////////////////////////////////////////////////
