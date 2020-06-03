#include "pch.h"
#include "WallSection.h"
#include "TerrainTile.h"

WallSection::WallSection(GenericRoom* room)
    : mRoom(room)
    , mFaceDirection(eDirection_N)
    , mFaceId(eTileFace_SideN)
{
}

void WallSection::Setup(eDirection faceDirection)
{
    mFaceDirection = faceDirection;
    mFaceId = DirectionToFaceId(faceDirection);
}

void WallSection::Setup(eTileFace faceId)
{
    mFaceDirection = FaceIdToDirection(faceId);
    mFaceId = faceId;
}

void WallSection::RemoveTile(TerrainTile* terrainTile)
{
    if (terrainTile)
    {
        cxx::erase_elements(mMapTiles, terrainTile);
    }
    else
    {
        debug_assert(false);
    }
}

void WallSection::InsertTileHead(TerrainTile* terrainTile)
{
    if (terrainTile)
    {
        debug_assert(!cxx::contains(mMapTiles, terrainTile));
        mMapTiles.insert(mMapTiles.begin(), terrainTile);
    }
    else
    {
        debug_assert(false);
    }
}

void WallSection::InsertTileTail(TerrainTile* terrainTile)
{
    if (terrainTile)
    {
        debug_assert(!cxx::contains(mMapTiles, terrainTile));
        mMapTiles.push_back(terrainTile);
    }
    else
    {
        debug_assert(false);
    }
}

void WallSection::RemoveTiles()
{
    mMapTiles.clear();
}

bool WallSection::ContainsTile(TerrainTile* terrainTile) const
{
    return cxx::contains(mMapTiles, terrainTile);
}

bool WallSection::ContainsTiles() const
{
    return !mMapTiles.empty();
}

bool WallSection::IsInnerTile(TerrainTile* terrainTile) const
{
    debug_assert(terrainTile);
    if (IsHeadTile(terrainTile) || IsTailTile(terrainTile))
        return false;

    return true;
}

bool WallSection::IsOuterTile(TerrainTile* terrainTile) const
{
    debug_assert(terrainTile);
    if (IsHeadTile(terrainTile) || IsTailTile(terrainTile))
        return true;

    return false;
}

bool WallSection::IsHeadTile(TerrainTile* terrainTile) const
{
    if (mMapTiles.empty())
    {
        debug_assert(false);
        return false;
    }

    if (mFaceDirection == eDirection_S || mFaceDirection == eDirection_W)
    {
        return terrainTile == mMapTiles.back();
    }
    else
    {
        return terrainTile == mMapTiles.front();
    }
    return false;
}

bool WallSection::IsTailTile(TerrainTile* terrainTile) const
{
    if (mMapTiles.empty())
    {
        debug_assert(false);
        return false;
    }

    if (mFaceDirection == eDirection_S || mFaceDirection == eDirection_W)
    {
        return terrainTile == mMapTiles.front();
    }
    else
    {
        return terrainTile == mMapTiles.back();
    }
    return false;
}

bool WallSection::IsEvenTile(TerrainTile* terrainTile) const
{
    bool isEven = false;
    if (terrainTile)
    {
        if (mFaceDirection == eDirection_N || mFaceDirection == eDirection_S)
        {
            isEven = cxx::is_even(terrainTile->mTileLocation.x);
        }
        else
        {
            isEven = cxx::is_even(terrainTile->mTileLocation.y);
        }
    }
    else
    {
        debug_assert(false);
    }
    return isEven;
}
