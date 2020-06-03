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
    void InsertTileHead(TerrainTile* terrainTile);
    void InsertTileTail(TerrainTile* terrainTile);
    void RemoveTile(TerrainTile* terrainTile);
    void RemoveTiles();

    bool ContainsTile(TerrainTile* terrainTile) const;
    bool ContainsTiles() const;

    bool IsInnerTile(TerrainTile* terrainTile) const;
    bool IsOuterTile(TerrainTile* terrainTile) const;

    // if face direction is North or South then
    //  - Head is on West
    //  - Tail is on East

    // if face direction in West or East then
    //  - Head is on North
    //  - Tail is on South

    bool IsHeadTile(TerrainTile* terrainTile) const;
    bool IsTailTile(TerrainTile* terrainTile) const;

    bool IsEvenTile(TerrainTile* terrainTile) const;

public:
    GenericRoom* mRoom = nullptr;
    eDirection mFaceDirection;
    eTileFace mFaceId;
    TilesList mMapTiles;
};