#pragma once

#include "GameDefs.h"
#include "GameWorldDefs.h"
#include "MapTile.h"

// simple container of map blocks that make up a single wall segment of a room

class RoomWallSection: public cxx::noncopyable
{
public:
    RoomWallSection() = default;
    void Reset();

    void Configure(eDirection faceDirection);
    void Configure(eTileFace tileFace);

    inline eDirection GetDirection() const { return mFaceDirection; }
    inline eTileFace GetFace() const { return mTileFace; }

    inline cxx::span<MapTile*> GetTiles() const { return mMapTiles; }

    // add tile to wall section or remove tile
    void AppendHead(MapTile* wallTile);
    void AppendTail(MapTile* wallTile);
    void RemoveTile(MapTile* wallTile);
    void RemoveTiles();

    bool ContainsTile(MapTile* wallTile) const;
    bool ContainsTiles() const;

    int GetLength() const;

    inline bool IsHorz() const { return (mFaceDirection == eDirection_N) || (mFaceDirection == eDirection_S); }
    inline bool IsVert() const { return (mFaceDirection == eDirection_E) || (mFaceDirection == eDirection_W); } 

    bool IsInner(MapTile* wallTile) const;

    // if face direction is North or South then
    //  - Head is on West
    //  - Tail is on East

    // if face direction in West or East then
    //  - Head is on North
    //  - Tail is on South

    bool IsHead(MapTile* wallTile) const;
    bool IsTail(MapTile* wallTile) const;

    bool IsOdd(MapTile* wallTile) const;

private:
    eDirection mFaceDirection = eDirection_COUNT;

    eTileFace mTileFace = eTileFace_COUNT;
    //
    //        Head     Inner Tiles    Tail
    //       ______  ______  ______  ______
    //      |      ||      ||      ||      |
    //      |______||______||______||______|        <- Wall blocks
    //      |___S__||___S__||___S__||___S__|        <- Tile face : S, Face direction : S
    //       //////  //////  //////  //////
    //       \\\\\\  \\\\\\  \\\\\\  \\\\\\         <- Room tiles
    //       //////  //////  //////  //////
    //

    std::vector<MapTile*> mMapTiles;
};
