#pragma once

#include "GameDefs.h"
#include "GameMapDefs.h"
#include "ScenarioDefs.h"

// Tile transformations used during geometry building

// Frequently used tile rotations
// 0: -90 degrees
// 1: +90 degrees
// 2: -180 degrees
// 3: -120 degrees
// 4: +120 degress
extern const glm::mat3 g_TileRotations[5];

// Subtile piece translations, quads
// 0: TOP-LEFT (NW)
// 1: TOP-RIGHT (NE)
// 2: BOTTOM-RIGHT (SE)
// 3: BOTTOM-LEFT (SW)
extern const glm::vec3 g_SubTileTranslations[4];

//////////////////////////////////////////////////////////////////////////

// Tile face geometry piece
struct TileFaceMesh
{
public:
    struct Piece
    {
    public:
        SurfaceMaterial mMaterial;
        unsigned int mBaseVertex = 0;
        unsigned int mTrianglesOffset = 0;
        unsigned int mVertexCount = 0;
        unsigned int mTriangleCount = 0;
    };
public:
    std::vector<glm::ivec3> mTriangles;
    std::vector<TerrainVertex3D> mVertices;
    std::vector<Piece> mPieces;
};

//////////////////////////////////////////////////////////////////////////

// Tile face
struct TileFaceData 
{
public:
    TileFaceData() = default;

public:
    TileFaceMesh mFaceMesh;

    bool mFaceMeshDirty = false; // face geometry must be rebuilt

    // data belove is only specified when tile side is part of the room
    // only defined for sides N, E, S, W
    bool mWallExtendsRoom = false;
};

//////////////////////////////////////////////////////////////////////////

struct TileHeightmap
{
public:
    TileHeightmap() = default;
public:
    static const int Resolution = 4;
    float mSamples[Resolution * Resolution] {};
};

//////////////////////////////////////////////////////////////////////////
// Information about map tile block
//////////////////////////////////////////////////////////////////////////

class MapTile
{
public:
    MapTile();

    // Test whether tile base terrain type is water or lava
    inline bool IsBaseTerrainWaterOrLava() const 
    { 
        const TerrainDefinition* tileBaseTerrain = GetBaseTerrain();
        return tileBaseTerrain->mIsLava || tileBaseTerrain->mIsWater; 
    }

    // Test whether tile is solid block (ie has walls)
    inline bool IsTerrainSolid() const 
    { 
        const TerrainDefinition* tileTerrain = GetTerrain();
        return tileTerrain->mIsSolid; 
    }

    // Test whether walls could be overriden by rooms
    inline bool IsTerrainAllowRoomWalls() const 
    { 
        const TerrainDefinition* tileTerrain = GetTerrain();
        return tileTerrain->mAllowRoomWalls; 
    }

    // Test whether some of tile faces has dirty geometry
    inline bool HasDirtyFaceMesh() const
    {
        for (const TileFaceData& roller: mFaces) 
        { 
            if (roller.mFaceMeshDirty) return true; 
        }
        return false;
    }

    // Test whether map tiles has same terrain type
    inline bool SameTileTerrainType(const MapTile* targetTile) const
    {
        if (targetTile == nullptr)
            return false;

        const TerrainDefinition* thisTerrain = GetTerrain();
        const TerrainDefinition* thatTerrain = targetTile->GetTerrain();
        return thisTerrain == thatTerrain;
    }

    // Test whether tile has same room instance
    inline bool SameRoomInstance(const MapTile* targetTile) const
    {
        return mRoomInstance && targetTile && (mRoomInstance == targetTile->mRoomInstance);
    }

    // Test whether map tiles has same base terrain type
    inline bool SameTileBaseTerrainType(const MapTile* mapTile) const
    {
        return mapTile && (mBaseTerrain == mapTile->mBaseTerrain);
    }

    // Test whether neighbour tile has same terrain type
    inline bool SameNeighbourTerrainType(eDirection neighbour) const 
    {
        cxx_assert(neighbour < eDirection_COUNT);
        return SameTileTerrainType(mNeighbours[neighbour]);
    }

    // Test whether neighbour tile has same base terrain type
    inline bool SameNeighbourBaseTerrainType(eDirection neighbour) const 
    {
        cxx_assert(neighbour < eDirection_COUNT);
        return SameTileBaseTerrainType(mNeighbours[neighbour]);
    }

    // Test whether tile has neighbour at specified direction
    inline bool HasNeighbour(eDirection neighbour) const 
    {
        cxx_assert(neighbour < eDirection_COUNT);
        return mNeighbours[neighbour] != nullptr;
    }

    // Test whether neighbour tile has same room instance
    inline bool SameNeighbourRoomInstance(eDirection neighbour) const 
    {
        cxx_assert(neighbour < eDirection_COUNT);
        return SameRoomInstance(mNeighbours[neighbour]);
    }

    inline bool SameNeighbourOwner(eDirection direction) const
    {
        cxx_assert(direction < eDirection_COUNT);
        return mNeighbours[direction] && (mNeighbours[direction]->mOwnerID == mOwnerID);
    }

    // Test whether neighbour tile is exists and it is solid
    inline bool NeighbourTileSolid(eDirection direction) const
    {
        return mNeighbours[direction] && mNeighbours[direction]->IsTerrainSolid();
    }

    // Get base terrain type
    inline TerrainDefinition* GetBaseTerrain() const { return mBaseTerrain; }

    // Get overriden terrain type
    inline TerrainDefinition* GetTerrain() const 
    {
        return mTerrain ? mTerrain : mBaseTerrain;
    }

    // Get tile side by direction
    // @param direction: Direction, must be one of N,E,S,W
    inline TileFaceData* GetTileFace(eDirection direction)
    {
        cxx_assert(IsStraightDirection(direction));
        switch (direction)
        {
            case eDirection_N: return &mFaces[eTileFace_SideN];
            case eDirection_E: return &mFaces[eTileFace_SideE];
            case eDirection_S: return &mFaces[eTileFace_SideS];
            case eDirection_W: return &mFaces[eTileFace_SideW];
        }
        return nullptr;
    }
    inline const TileFaceData* GetTileFace(eDirection direction) const
    {
        cxx_assert(IsStraightDirection(direction));
        switch (direction)
        {
            case eDirection_N: return &mFaces[eTileFace_SideN];
            case eDirection_E: return &mFaces[eTileFace_SideE];
            case eDirection_S: return &mFaces[eTileFace_SideS];
            case eDirection_W: return &mFaces[eTileFace_SideW];
        }
        cxx_assert(false);
        return nullptr;
    }

    inline bool IsWallExtendsRoom(eDirection direction) const
    {
        const TileFaceData* tileface = GetTileFace(direction);
        return tileface && tileface->mWallExtendsRoom;
    }

    // Reset mesh geometries for specified tile face
    void ClearTileMesh(eTileFace meshFace);

    // Reset mesh geometries for all tile faces
    void ClearTileMesh();

    // set floor heightmap data to default floor level
    void ClearFloorHeightmap();

public:
    MapPoint2D mTileLocation; // logical location of tile

    TerrainDefinition* mBaseTerrain; // used to determine base terrain type, cannot be null
    TerrainDefinition* mTerrain; // override base terrain type, can be null

    ePlayerID mOwnerID;

    Room* mRoomInstance; // room built on that tile

    MapTile* mNeighbours[eDirection_COUNT];

    TileFaceData mFaces[eTileFace_COUNT];
    TileHeightmap mFloorHeightmap;

    unsigned int mRandomValue = 0; // affects on visuals only
    unsigned int mFloodFillCounter = 0; // gets modified on each flood fill operation

    // these flags is valid only if tile is a part of room
    bool mIsTagged;
    bool mIsRoomInnerTile; // tile is center of 3x3 square of room
    bool mIsRoomEntrance;
}; 