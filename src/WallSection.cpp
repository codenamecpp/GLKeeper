#include "pch.h"
#include "WallSection.h"
#include "MapTile.h"

WallSection::WallSection(GenericRoom* room, eDirection direction)
    : mRoom(room)
    , mFaceDirection(direction)
{
    mFaceId = DirectionToTileFace(direction);
}

WallSection::WallSection(GenericRoom* room, eTileFace faceid)
    : mRoom(room)
    , mFaceId(faceid)
{
    mFaceDirection = TileFaceToDirection(faceid);
}

void WallSection::RemoveTile(MapTile* mapTile)
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

void WallSection::InsertTileHead(MapTile* mapTile)
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

void WallSection::InsertTileTail(MapTile* mapTile)
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

bool WallSection::ContainsTile(MapTile* mapTile) const
{
    return cxx::contains(mMapTiles, mapTile);
}

bool WallSection::ContainsTiles() const
{
    return !mMapTiles.empty();
}

bool WallSection::IsInnerTile(MapTile* mapTile) const
{
    debug_assert(mapTile);
    if (IsHeadTile(mapTile) || IsTailTile(mapTile))
        return false;

    return true;
}

bool WallSection::IsOuterTile(MapTile* mapTile) const
{
    debug_assert(mapTile);
    if (IsHeadTile(mapTile) || IsTailTile(mapTile))
        return true;

    return false;
}

bool WallSection::IsHeadTile(MapTile* mapTile) const
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

bool WallSection::IsTailTile(MapTile* mapTile) const
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

bool WallSection::IsEvenTile(MapTile* mapTile) const
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
