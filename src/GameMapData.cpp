#include "pch.h"
#include "GameMapData.h"
#include "randomizer.h"

void GameMapData::Setup(const Size2D& mapDimensions, unsigned int randomSeed)
{
    Clear();

    mDimensions = mapDimensions;
    mMapRandomSeed = randomSeed;

    mTilesArray.resize(mapDimensions.x * mapDimensions.y);
    // initialize tiles
    // use random number generator for tile visuals diversity
    cxx::randomizer randomize(randomSeed);

    for (int tiley = 0; tiley < mDimensions.y; ++tiley)
    for (int tilex = 0; tilex < mDimensions.x; ++tilex)
    {
        const int tileIndex = (tiley * mDimensions.x) + tilex;
            
        GameMapTile& currentTile = mTilesArray[tileIndex];
        currentTile.mRandomValue = randomize.generate_int();
        currentTile.mTileLocation.x = tilex;
        currentTile.mTileLocation.y = tiley;
    }

    for (GameMapTile& currTile: mTilesArray)
    {
        currTile.mRandomValue = randomize.generate_int();
    }
}

void GameMapData::Clear()
{
    mFloodFillCounter = 0;
    mDimensions.x = 0;
    mDimensions.y = 0;
    mTilesArray.clear();
}

GameMapTile* GameMapData::GetTileFromCoord3d(const glm::vec3& coord)
{
    Point2D tileLocation {
        static_cast<int>((coord.x + DUNGEON_CELL_HALF_SIZE) / DUNGEON_CELL_SIZE),
        static_cast<int>((coord.z + DUNGEON_CELL_HALF_SIZE) / DUNGEON_CELL_SIZE)
    };
    return GetMapTile(tileLocation);
}

GameMapTile* GameMapData::GetNeighbourTile(const GameMapTile* mapTile, eDirection direction)
{
    debug_assert(mapTile);

    return GetMapTile(mapTile->mTileLocation, direction);
}

GameMapTile* GameMapData::GetMapTile(const Point2D& tileLocation)
{
    if (IsWithinMap(tileLocation))
        return &mTilesArray[tileLocation.y * mDimensions.x + tileLocation.x];

    return nullptr;
}

GameMapTile* GameMapData::GetMapTile(const Point2D& tileLocation, eDirection direction)
{
    Point2D directionVector = GetDirectionVector(direction);

    return GetMapTile(tileLocation + directionVector);
}

bool GameMapData::IsTileHasNeighbour(const GameMapTile* mapTile, eDirection direction) const
{
    debug_assert(mapTile);

    return IsWithinMap(mapTile->mTileLocation, direction);
}

bool GameMapData::GetTileCenterCoord3d(const Point2D& tileLocation, glm::vec3& coord3d) const
{
    bool isWithin = IsWithinMap(tileLocation);
    if (isWithin)
    {
        coord3d.x = tileLocation.x * DUNGEON_CELL_SIZE;
        coord3d.y = 1.0f;
        coord3d.z = tileLocation.y * DUNGEON_CELL_SIZE;
    }
    return isWithin;
}

bool GameMapData::IsWithinMap(const Point2D& tileLocation, eDirection direction) const
{
    Point2D directionVector = GetDirectionVector(direction);

    Point2D nextTilePosition = tileLocation + directionVector;
    return (nextTilePosition.x > -1) && (nextTilePosition.y > -1) && 
        (nextTilePosition.x < mDimensions.x) && 
        (nextTilePosition.y < mDimensions.y); 
}
