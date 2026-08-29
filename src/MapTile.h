#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameDefs.h"
#include "GameMapDefs.h"
#include "ScenarioDefs.h"
#include "PlayerDefs.h"

//////////////////////////////////////////////////////////////////////////

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

    // Whether tile base terrain type is water or lava
    inline bool IsBaseTerrainWaterOrLava() const 
    { 
        const TerrainDefinition* tileBaseTerrain = GetBaseTerrain();
        return tileBaseTerrain->mIsLava || tileBaseTerrain->mIsWater; 
    }

    // Whether tile is solid block (ie has walls)
    inline bool IsSolidBlock() const 
    { 
        const TerrainDefinition* tileTerrain = GetTerrain();
        return tileTerrain->mIsSolid; 
    }

    inline bool IsImpenetrable() const
    {
        const TerrainDefinition* tileTerrain = GetTerrain();
        return tileTerrain->mIsImpenetrable;
    }

    // Test whether walls could be overriden by rooms
    inline bool IsAllowRoomWalls() const 
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

    inline bool HasOwner(ePlayerID playerId) const { return mOwnerId == playerId; }

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
        return mNeighbours[direction] && mNeighbours[direction]->HasOwner(mOwnerId);
    }

    // Test whether neighbour tile is exists and it is solid
    inline bool NeighbourTileSolid(eDirection direction) const
    {
        return mNeighbours[direction] && mNeighbours[direction]->IsSolidBlock();
    }

    inline TerrainDefinition* GetBaseTerrain() const { return mBaseTerrain; }
    inline TerrainDefinition* GetTerrain() const 
    {
        return mRoomTerrain ? mRoomTerrain : mBaseTerrain;
    }

    void SetBaseTerrain(TerrainDefinition* terrainDef);
    void SetRoomTerrain(TerrainDefinition* terrainDef);

    inline int GetHitPoints() const { return mHitPoints; }
    inline int GetHitPointsMax() const 
    {
        const TerrainDefinition* terrainDef = GetTerrain();
        return terrainDef->mHealthMax;
    }
    int ChangeHitPoints(int deltaHitpoints);
    void RestoreHitPoints();

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

    inline bool IsTaggedForDigging(ePlayerID playerId) const
    {
        return mTaggedByPlayers.Contains(playerId);
    }

    inline void SetTaggedForDigging(ePlayerID playerId, bool isTagged)
    {
        if (isTagged) { mTaggedByPlayers.Include(playerId); } else { mTaggedByPlayers.Exclude(playerId); }
    }

    inline MapAreaCode GetAreaCode(ePassabilityType passabilityType) const
    {
        cxx_assert(passabilityType < ePassabilityType_COUNT);
        return mAreaCode[passabilityType];
    }

    // Reset mesh geometries for specified tile face
    void ClearTileMesh(eTileFace meshFace);

    // Reset mesh geometries for all tile faces
    void ClearTileMesh();

    // set floor heightmap data to default floor level
    void ClearFloorHeightmap();

    void ClearAreaCode();

private:
    TerrainDefinition* mBaseTerrain; // base terrain type, cannot be null
    TerrainDefinition* mRoomTerrain; // overrides the base terrain type when placing a room on the tile, optional

public:
    MapTile* mNeighbours[eDirection_COUNT];

    Room* mRoomInstance; // room built on that tile

    ePlayerID mOwnerId;

    Point2D mLocation; // logical tile coordinate
    TileFaceData mFaces[eTileFace_COUNT];
    TileHeightmap mFloorHeightmap;

    unsigned int mRandomValue = 0; // affects on visuals only
    unsigned int mFloodFillCounter = 0; // gets modified on each flood fill operation
    int mHitPoints = 100;

    MapAreaCode mAreaCode[ePassabilityType_COUNT];

    PlayerIdSet mTaggedByPlayers;

    // these flags is valid only if tile is a part of room
    bool mIsRoomInnerTile; // tile is center of 3x3 square of room
    bool mIsRoomEntrance;    
}; 
