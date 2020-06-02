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

void WallSection::RemoveTile(TerrainTile* mapTile)
{
    if (mapTile)
    {
        cxx::erase_elements(mMapTiles, mapTile);
    }
    else
    {
        debug_assert(false);
    }
}

void WallSection::InsertTileHead(TerrainTile* mapTile)
{
    if (mapTile)
    {
        debug_assert(!cxx::contains(mMapTiles, mapTile));
        mMapTiles.insert(mMapTiles.begin(), mapTile);
    }
    else
    {
        debug_assert(false);
    }
}

void WallSection::InsertTileTail(TerrainTile* mapTile)
{
    if (mapTile)
    {
        debug_assert(!cxx::contains(mMapTiles, mapTile));
        mMapTiles.push_back(mapTile);
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

bool WallSection::ContainsTile(TerrainTile* mapTile) const
{
    return cxx::contains(mMapTiles, mapTile);
}

bool WallSection::ContainsTiles() const
{
    return !mMapTiles.empty();
}

bool WallSection::IsInnerTile(TerrainTile* mapTile) const
{
    debug_assert(mapTile);
    if (IsHeadTile(mapTile) || IsTailTile(mapTile))
        return false;

    return true;
}

bool WallSection::IsOuterTile(TerrainTile* mapTile) const
{
    debug_assert(mapTile);
    if (IsHeadTile(mapTile) || IsTailTile(mapTile))
        return true;

    return false;
}

bool WallSection::IsHeadTile(TerrainTile* mapTile) const
{
    if (mMapTiles.empty())
    {
        debug_assert(false);
        return false;
    }

    if (mFaceDirection == eDirection_S || mFaceDirection == eDirection_W)
    {
        return mapTile == mMapTiles.back();
    }
    else
    {
        return mapTile == mMapTiles.front();
    }
    return false;
}

bool WallSection::IsTailTile(TerrainTile* mapTile) const
{
    if (mMapTiles.empty())
    {
        debug_assert(false);
        return false;
    }

    if (mFaceDirection == eDirection_S || mFaceDirection == eDirection_W)
    {
        return mapTile == mMapTiles.front();
    }
    else
    {
        return mapTile == mMapTiles.back();
    }
    return false;
}

bool WallSection::IsEvenTile(TerrainTile* mapTile) const
{
    bool isEven = false;
    if (mapTile)
    {
        if (mFaceDirection == eDirection_N || mFaceDirection == eDirection_S)
        {
            isEven = cxx::is_even(mapTile->mTileLocation.x);
        }
        else
        {
            isEven = cxx::is_even(mapTile->mTileLocation.y);
        }
    }
    else
    {
        debug_assert(false);
    }
    return isEven;
}
