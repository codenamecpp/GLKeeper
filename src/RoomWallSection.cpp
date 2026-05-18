#include "stdafx.h"
#include "RoomWallSection.h"

void RoomWallSection::Reset()
{
    mFaceDirection = eDirection_COUNT;
    mTileFace = eTileFace_COUNT;
    mMapTiles.clear();
}

void RoomWallSection::Configure(eDirection faceDirection)
{
    cxx_assert(faceDirection < eDirection_COUNT);
    mFaceDirection = faceDirection;
    mTileFace = DirectionToTileFace(faceDirection);
    cxx_assert(mTileFace < eTileFace_COUNT);
}

void RoomWallSection::Configure(eTileFace tileFace)
{
    mFaceDirection = TileFaceToDirection(tileFace);
    cxx_assert(mFaceDirection < eDirection_COUNT);
    mTileFace = tileFace;
    cxx_assert(mTileFace < eTileFace_COUNT);
}

void RoomWallSection::AppendHead(MapTile* wallTile)
{
    bool isSuccess = wallTile && !ContainsTile(wallTile);
    cxx_assert(isSuccess);
    if (isSuccess)
    {
        if ((mFaceDirection == eDirection_S) || (mFaceDirection == eDirection_W))
        {
            mMapTiles.push_back(wallTile);
        }
        else
        {
            mMapTiles.insert(mMapTiles.begin(), wallTile);
        }
    }
}

void RoomWallSection::AppendTail(MapTile* wallTile)
{
    bool isSuccess = wallTile && !ContainsTile(wallTile);
    cxx_assert(isSuccess);
    if (isSuccess)
    {
        if ((mFaceDirection == eDirection_S) || (mFaceDirection == eDirection_W))
        {
            mMapTiles.insert(mMapTiles.begin(), wallTile);
        }
        else
        {
            mMapTiles.push_back(wallTile);
        }
    }
}

void RoomWallSection::RemoveTile(MapTile* wallTile)
{
    cxx_assert(wallTile);
    if (!cxx::erase(mMapTiles, wallTile))
    {
        cxx_assert(false);
    }
}

void RoomWallSection::RemoveTiles()
{
    mMapTiles.clear();
}

bool RoomWallSection::ContainsTile(MapTile* wallTile) const
{
    return wallTile && cxx::contains(mMapTiles, wallTile);
}

bool RoomWallSection::ContainsTiles() const
{
    return !mMapTiles.empty();
}

int RoomWallSection::GetLength() const
{
    return static_cast<int>(mMapTiles.size());
}

bool RoomWallSection::IsInner(MapTile* wallTile) const
{
    return !(IsHead(wallTile) || IsTail(wallTile));
}

bool RoomWallSection::IsHead(MapTile* wallTile) const
{
    if (mMapTiles.empty())
    {
        cxx_assert(false);
        return false;
    }

    if ((mFaceDirection == eDirection_S) || (mFaceDirection == eDirection_W))
    {
        return wallTile == mMapTiles.back();
    }
    else
    {
        return wallTile == mMapTiles.front();
    }
}

bool RoomWallSection::IsTail(MapTile* wallTile) const
{
    if (mMapTiles.empty())
    {
        cxx_assert(false);
        return false;
    }

    if ((mFaceDirection == eDirection_S) || (mFaceDirection == eDirection_W))
    {
        return wallTile == mMapTiles.front();
    }
    else
    {
        return wallTile == mMapTiles.back();
    }
}

bool RoomWallSection::IsOdd(MapTile* wallTile) const
{
    cxx_assert(wallTile);
    return cxx::is_odd(wallTile->mTileLocation.x + wallTile->mTileLocation.y);
}
