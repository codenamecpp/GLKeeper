#pragma once

#include "GameDefs.h"
#include "GameMapTile.h"

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
class GameMapData final
{
public:
    // readonly
    Size2D mDimensions;

public:
    // setup map tiles
    void Setup(const Size2D& mapDimensions, unsigned int randomSeed);
    void Clear();

    // get map tile by world position
    // @param coordx, coordz: World coordinates
    GameMapTile* GetTileFromCoord3d(const glm::vec3& coord);

    // get neighbour tile for direction
    // @param mapTile: Tile
    // @param direction: Neighbour tile direction
    GameMapTile* GetNeighbourTile(const GameMapTile* mapTile, eDirection direction);

    // get map tile by logical position
    // @param tileLocation: Tile logical position
    GameMapTile* GetMapTile(const Point2D& tileLocation);
    GameMapTile* GetMapTile(const Point2D& tileLocation, eDirection direction);

    // test whether tile has neighbour at specified direction
    // @param mapTile: Tile
    // @param direction: Neighbour tile direction
    bool IsTileHasNeighbour(const GameMapTile* mapTile, eDirection direction) const;

    // test whether tile position is within map
    // @param tileLocation: Tile logical position
    inline bool IsWithinMap(const Point2D& tileLocation) const 
    {
        return (tileLocation.x > -1) && (tileLocation.y > -1) && 
            (tileLocation.x < mDimensions.x) && 
            (tileLocation.y < mDimensions.y); 
    }

    // test whether next tile position is within map
    bool IsWithinMap(const Point2D& tileLocation, eDirection direction) const;

    // get tile center world coordinate
    // @param tileLocation: Tile location
    // @param coord3d: Output world coordinate
    bool GetTileCenterCoord3d(const Point2D& tileLocation, glm::vec3& coord3d) const;

    // flood fill adjacent tiles in 4 directions
    // @param outputTiles: Result tile array including initial tile
    // @param origin: Initial tile
    // @param scanArea: Scanning bounds
    // @param floodFillFlags: Flags
    void FloodFill4(GameMapTiles& outputTiles, GameMapTile* origin, MapFloodFillFlags flags);
    void FloodFill4(GameMapTiles& outputTiles, GameMapTile* origin, const Rect2D& scanArea, MapFloodFillFlags flags);

private:
    void ClearFloodFillCounter();

private:
    std::vector<GameMapTile> mTilesArray;

    unsigned int mMapRandomSeed = 0;
    unsigned int mFloodFillCounter = 0; // increments on each flood fill operation
};
