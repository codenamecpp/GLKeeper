#pragma once

#include "GameDefs.h"
#include "ScenarioDefs.h"

// forwards
struct RoomWallSection;

// frequently used tile rotations
// 0: -90 degrees
// 1: +90 degrees
// 2: -180 degrees
// 3: -120 degrees
// 4: +120 degress
extern const glm::mat3 g_TileRotations[5];

// subtile piece translations, quads
// 0: TOP-LEFT (NW)
// 1: TOP-RIGHT (NE)
// 2: BOTTOM-RIGHT (SE)
// 3: BOTTOM-LEFT (SW)
extern const glm::vec3 g_SubTileTranslations[4];

// tile face geometry
struct TileMesh
{
public:
    RenderMaterial mMaterial;

    std::vector<glm::ivec3> mTriangles;
    std::vector<Vertex3D_Terrain> mVertices;
};

// tile face data
struct TileFaceData 
{
public:
    std::vector<TileMesh> mMeshArray;

    // specified if tile face is part of the room
    // only defined for sides N, E, S, W 
    RoomWallSection* mWallSectionData = nullptr;                                               

    bool mInvalidated = false; // face geometry is dirty and must be rebuilt
};

// gamemap block data
class MapTile
{
public:
    MapTile();

    // reset mesh geometries for specific tile face
    // @param faceid: Face index
    void ClearTileMesh(eTileFace faceid);
    void ClearTileMesh();

    // get current terrain type for tile
    inline TerrainDefinition* GetTerrain() const { return mRoomTerrain ? mRoomTerrain : mBaseTerrain; }
    inline TerrainDefinition* GetBaseTerrain() const { return mBaseTerrain; }

public:
    Point2D mTileLocation; // logical tile location 2D

    TerrainDefinition* mBaseTerrain = nullptr; // used to determine base terrain type, cannot be null
    TerrainDefinition* mRoomTerrain = nullptr; // overrides base terrain with room specific terrain, optional

    ePlayerID mOwnerId = ePlayerID_Null;

    GenericRoom* mRoomInstance = nullptr; // room built on tile
    TileFaceData mFaces[eTileFace_COUNT];
    MapTile* mNeighbours[eDirection_COUNT];

    unsigned int mRandomValue = 0; // effects on visuals only
    unsigned int mFloodFillCounter = 0; // increments on each flood fill operation

    bool mIsTagged;
    bool mIsRoomInnerTile; // tile is center of 3x3 square of room, flag is valid only if tile is a part of room
    bool mIsRoomEntrance; // flag is valid only if tile is a part of room
};