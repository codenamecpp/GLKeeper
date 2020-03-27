#pragma once

#include "GameDefs.h"
#include "RoomsDefs.h"

// wall section which belongs to room
class RoomWallSection
{
public:
    // readonly
    GenericRoom* mRoom = nullptr;

    eDirection mFaceDirection;
    eTileFace mFaceId;

    TilesArray mWallTiles;

public:
    void Setup(GenericRoom* room, eDirection direction);
    void Setup(GenericRoom* room, eTileFace faceid);

    void DetachTiles();
    void DetachTile(MapTile* mapTile);
    void AttachTile(MapTile* mapTile);

    bool ContainsTile(MapTile* mapTile) const;
    bool ContainsTiles() const;

    bool IsInnerTile(MapTile* mapTile) const;
    bool IsOuterTile(MapTile* mapTile) const;

    // if face direction is North or South then
    //  - Head is on West
    //  - Tail is on East

    // if face direction in West or East then
    //  - Head is on North
    //  - Tail is on South

    bool IsHeadTile(MapTile* mapTile) const;
    bool IsTailTile(MapTile* mapTile) const;

    MapTile* GetTailTile() const;
    MapTile* GetHeadTile() const;
};