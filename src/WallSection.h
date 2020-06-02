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
    void InsertTileHead(TerrainTile* mapTile);
    void InsertTileTail(TerrainTile* mapTile);
    void RemoveTile(TerrainTile* mapTile);
    void RemoveTiles();

    bool ContainsTile(TerrainTile* mapTile) const;
    bool ContainsTiles() const;

    bool IsInnerTile(TerrainTile* mapTile) const;
    bool IsOuterTile(TerrainTile* mapTile) const;

    // if face direction is North or South then
    //  - Head is on West
    //  - Tail is on East

    // if face direction in West or East then
    //  - Head is on North
    //  - Tail is on South

    bool IsHeadTile(TerrainTile* mapTile) const;
    bool IsTailTile(TerrainTile* mapTile) const;

    bool IsEvenTile(TerrainTile* mapTile) const;

public:
    GenericRoom* mRoom = nullptr;
    eDirection mFaceDirection;
    eTileFace mFaceId;
    TilesArray mMapTiles;
};