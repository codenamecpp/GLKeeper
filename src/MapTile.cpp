#include "stdafx.h"
#include "MapTile.h"

// Rotations Y
const glm::mat3 g_TileRotations[5] = 
{
    glm::mat3(glm::rotate(glm::radians( -90.0f), glm::vec3{0.0f, 1.0f, 0.0f})), // 0
    glm::mat3(glm::rotate(glm::radians(  90.0f), glm::vec3{0.0f, 1.0f, 0.0f})), // 1
    glm::mat3(glm::rotate(glm::radians(-180.0f), glm::vec3{0.0f, 1.0f, 0.0f})), // 2
    glm::mat3(glm::rotate(glm::radians(-120.0f), glm::vec3{0.0f, 1.0f, 0.0f})), // 3
    glm::mat3(glm::rotate(glm::radians( 120.0f), glm::vec3{0.0f, 1.0f, 0.0f})), // 4
};

// Quads translations
const glm::vec3 g_SubTileTranslations[4] = 
{
    {-MAP_TILE_HALF_SIZE * 0.5f, 0.0f, -MAP_TILE_HALF_SIZE * 0.5f}, // TOP-LEFT (NW)
    { MAP_TILE_HALF_SIZE * 0.5f, 0.0f, -MAP_TILE_HALF_SIZE * 0.5f}, // TOP-RIGHT (NE)
    { MAP_TILE_HALF_SIZE * 0.5f, 0.0f,  MAP_TILE_HALF_SIZE * 0.5f}, // BOTTOM-RIGHT (SE)
    {-MAP_TILE_HALF_SIZE * 0.5f, 0.0f,  MAP_TILE_HALF_SIZE * 0.5f}, // BOTTOM-LEFT (SW)
};

MapTile::MapTile()
    : mBaseTerrain()
    , mTerrain()
    , mOwnerID(ePlayerID_Null)
    , mTileLocation()
    , mRoomInstance()
    , mNeighbours()
    , mFaces()
    , mIsTagged()
    , mIsRoomInnerTile()
    , mIsRoomEntrance()
{
}

void MapTile::ClearTileMesh()
{
    for (TileFaceData& tileFace: mFaces)
    {
        tileFace.mFaceMesh.mPieces.clear();
        tileFace.mFaceMesh.mTriangles.clear();
        tileFace.mFaceMesh.mVertices.clear();
    }
}

void MapTile::ClearTileMesh(eTileFace meshFace)
{
    cxx_assert(meshFace < eTileFace_COUNT);
    if (meshFace < eTileFace_COUNT)
    {
        TileFaceData& tileFace = mFaces[meshFace];
        tileFace.mFaceMesh.mPieces.clear();
        tileFace.mFaceMesh.mTriangles.clear();
        tileFace.mFaceMesh.mVertices.clear();
    }
}

void MapTile::ClearFloorHeightmap()
{
    for (float& roller: mFloorHeightmap.mSamples)
    {
        roller = MAP_FLOOR_LEVEL;
    }
}
