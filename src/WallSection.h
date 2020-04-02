#pragma once

// wall section which belongs to room
class WallSection
{
public:
    WallSection(GenericRoom* room);

    // set wall section params but don't remove added tiles
    void Setup(eDirection faceDirection);
    void Setup(eTileFace faceId);

    // add tile to wall section or remove tile
    void InsertTileHead(MapTile* mapTile);
    void InsertTileTail(MapTile* mapTile);
    void RemoveTile(MapTile* mapTile);
    void RemoveTiles();

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

    bool IsEvenTile(MapTile* mapTile) const;

public:
    GenericRoom* mRoom = nullptr;
    eDirection mFaceDirection;
    eTileFace mFaceId;
    TilesArray mMapTiles;
};