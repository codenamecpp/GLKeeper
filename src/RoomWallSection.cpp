#include "pch.h"
#include "RoomWallSection.h"
#include "MapTile.h"

void RoomWallSection::Setup(GenericRoom* room, eDirection direction)
{
    debug_assert(mRoom == nullptr);
    room = mRoom;

    mFaceDirection = direction;
    mFaceId = DirectionToTileFace(direction);
}

void RoomWallSection::Setup(GenericRoom* room, eTileFace faceid)
{
    debug_assert(mRoom == nullptr);
    room = mRoom;

    mFaceDirection = TileFaceToDirection(faceid);
    mFaceId = faceid;
}

void RoomWallSection::DetachTile(MapTile* mapTile)
{
    if (mapTile == nullptr)
    {
        debug_assert(false);
        return;
    }

    TileFaceData& face = mapTile->mFaces[mFaceId];
    if (face.mRoomWallSection == this)
    {
        face.mRoomWallSection = nullptr;
    }

    cxx::erase_elements(mWallTiles, mapTile);
}

void RoomWallSection::AttachTile(MapTile* mapTile)
{
    if (mapTile == nullptr)
    {
        debug_assert(false);
        return;
    }

    TileFaceData& face = mapTile->mFaces[mFaceId];

    debug_assert(face.mRoomWallSection == nullptr);
    face.mRoomWallSection = this;

    cxx::push_back_if_unique(mWallTiles, mapTile);
}

void RoomWallSection::DetachTiles()
{
    for (MapTile* currentTile: mWallTiles)
    {
        TileFaceData& face = currentTile->mFaces[mFaceId];
        if (face.mRoomWallSection == this)
        {
            face.mRoomWallSection = nullptr;
        }
        else
        {
            debug_assert(false);
        }
    }
    mWallTiles.clear();
}

bool RoomWallSection::ContainsTile(MapTile* mapTile) const
{
    debug_assert(mapTile);
    return cxx::contains(mWallTiles, mapTile);
}

bool RoomWallSection::ContainsTiles() const
{
    return !mWallTiles.empty();
}

bool RoomWallSection::IsInnerTile(MapTile* mapTile) const
{
    debug_assert(mapTile);
    if (IsHeadTile(mapTile) || IsTailTile(mapTile))
        return false;

    return true;
}

bool RoomWallSection::IsOuterTile(MapTile* mapTile) const
{
    debug_assert(mapTile);
    if (IsHeadTile(mapTile) || IsTailTile(mapTile))
        return true;

    return false;
}

bool RoomWallSection::IsHeadTile(MapTile* mapTile) const
{
    if (mWallTiles.size())
    {
        return mapTile == mWallTiles.front();
    }

    return false;
}

bool RoomWallSection::IsTailTile(MapTile* mapTile) const
{
    if (mWallTiles.size())
    {
        return mapTile == mWallTiles.back();
    }

    return false;
}

MapTile* RoomWallSection::GetTailTile() const
{
    MapTile* tailMapTile = nullptr;
    if (mWallTiles.size())
    {
        tailMapTile = mWallTiles.back();
    }

    return tailMapTile;
}

MapTile* RoomWallSection::GetHeadTile() const
{
    MapTile* headMapTile = nullptr;
    if (mWallTiles.size())
    {
        headMapTile = mWallTiles.front();
    }

    return headMapTile;
}
