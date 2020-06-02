#pragma once

#include "TerrainTile.h"

// flood fill flags
struct MapFloodFillFlags
{
public:
    MapFloodFillFlags()
        : mSameOwner(true) // default
        , mSameBaseTerrain()
    {
    }
public:
    bool mSameOwner : 1; // match player id
    bool mSameBaseTerrain : 1; // ignore room terrain type
};

// game map data
class GameMap
{
public:
    // readonly
    Point mDimensions;
    cxx::aabbox mBounds;

public:
    // setup map tiles
    void Setup(const Point& mapDimensions, unsigned int randomSeed);
    void Clear();

    // get map tile by world position
    // @param coordx, coordz: World coordinates
    TerrainTile* GetTileFromCoord3d(const glm::vec3& coord);

    // get map tile by logical position
    // @param tileLocation: Tile logical position
    TerrainTile* GetMapTile(const Point& tileLocation);
    TerrainTile* GetMapTile(const Point& tileLocation, eDirection direction);

    // test whether tile position is within map
    // @param tileLocation: Tile logical position
    inline bool IsWithinMap(const Point& tileLocation) const 
    {
        return (tileLocation.x > -1) && (tileLocation.y > -1) && 
            (tileLocation.x < mDimensions.x) && 
            (tileLocation.y < mDimensions.y); 
    }

    // test whether next tile position is within map
    bool IsWithinMap(const Point& tileLocation, eDirection direction) const;

    // flood fill adjacent tiles in 4 directions
    // @param outputTiles: Result tile array including initial tile
    // @param origin: Initial tile
    // @param scanArea: Scanning bounds
    // @param floodFillFlags: Flags
    void FloodFill4(TilesArray& outputTiles, TerrainTile* origin, MapFloodFillFlags flags);
    void FloodFill4(TilesArray& outputTiles, TerrainTile* origin, const Rectangle& scanArea, MapFloodFillFlags flags);

    void ComputeBounds();

private:
    void ClearFloodFillCounter();

private:
    std::vector<TerrainTile> mTilesArray;

    unsigned int mMapRandomSeed = 0;
    unsigned int mFloodFillCounter = 0; // increments on each flood fill operation
};
