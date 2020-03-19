#include "pch.h"
#include "GameMapTile.h"

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
    {-DUNGEON_CELL_HALF_SIZE * 0.5f, 0.0f, -DUNGEON_CELL_HALF_SIZE * 0.5f}, // TOP-LEFT (NW)
    { DUNGEON_CELL_HALF_SIZE * 0.5f, 0.0f, -DUNGEON_CELL_HALF_SIZE * 0.5f}, // TOP-RIGHT (NE)
    { DUNGEON_CELL_HALF_SIZE * 0.5f, 0.0f,  DUNGEON_CELL_HALF_SIZE * 0.5f}, // BOTTOM-RIGHT (SE)
    {-DUNGEON_CELL_HALF_SIZE * 0.5f, 0.0f,  DUNGEON_CELL_HALF_SIZE * 0.5f}, // BOTTOM-LEFT (SW)
};

GameMapTile::GameMapTile()
    : mIsTagged()
    , mIsRoomInnerTile()
    , mIsRoomEntrance()
    , mTileLocation()
{
}

void GameMapTile::ClearTileMesh()
{
    for (TileFaceData& currFace: mFaces)
    {
        currFace.mMeshArray.clear();
    }
}

void GameMapTile::ClearTileMesh(eTileFace meshFace)
{
    debug_assert(meshFace < eTileFace_COUNT);
    if (meshFace < eTileFace_COUNT)
    {
        mFaces[meshFace].mMeshArray.clear();
    }
}