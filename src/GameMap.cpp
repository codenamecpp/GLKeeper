#include "pch.h"
#include "GameMap.h"
#include "randomizer.h"
#include <stack>

void GameMap::Setup(const Size2D& mapDimensions, unsigned int randomSeed)
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
            
        MapTile& currentTile = mTilesArray[tileIndex];
        currentTile.mRandomValue = randomize.generate_int();
        currentTile.mTileLocation.x = tilex;
        currentTile.mTileLocation.y = tiley;
        // setup neighbours
        currentTile.mNeighbours[eDirection_NW] = GetMapTile(currentTile.mTileLocation, eDirection_NW);
        currentTile.mNeighbours[eDirection_NE] = GetMapTile(currentTile.mTileLocation, eDirection_NE);
        currentTile.mNeighbours[eDirection_N]  = GetMapTile(currentTile.mTileLocation, eDirection_N);
        currentTile.mNeighbours[eDirection_W]  = GetMapTile(currentTile.mTileLocation, eDirection_W);
        currentTile.mNeighbours[eDirection_E]  = GetMapTile(currentTile.mTileLocation, eDirection_E);
        currentTile.mNeighbours[eDirection_SW] = GetMapTile(currentTile.mTileLocation, eDirection_SW);
        currentTile.mNeighbours[eDirection_SE] = GetMapTile(currentTile.mTileLocation, eDirection_SE);
        currentTile.mNeighbours[eDirection_S]  = GetMapTile(currentTile.mTileLocation, eDirection_S);
    }

    for (MapTile& currTile: mTilesArray)
    {
        currTile.mRandomValue = randomize.generate_int();
    }
}

void GameMap::Clear()
{
    mFloodFillCounter = 0;
    mDimensions.x = 0;
    mDimensions.y = 0;
    mTilesArray.clear();
}

MapTile* GameMap::GetTileFromCoord3d(const glm::vec3& coord)
{
    Point2D tileLocation {
        static_cast<int>((coord.x + DUNGEON_CELL_HALF_SIZE) / DUNGEON_CELL_SIZE),
        static_cast<int>((coord.z + DUNGEON_CELL_HALF_SIZE) / DUNGEON_CELL_SIZE)
    };
    return GetMapTile(tileLocation);
}

MapTile* GameMap::GetMapTile(const Point2D& tileLocation)
{
    if (IsWithinMap(tileLocation))
        return &mTilesArray[tileLocation.y * mDimensions.x + tileLocation.x];

    return nullptr;
}

MapTile* GameMap::GetMapTile(const Point2D& tileLocation, eDirection direction)
{
    Point2D directionVector = GetDirectionVector(direction);

    return GetMapTile(tileLocation + directionVector);
}

bool GameMap::IsWithinMap(const Point2D& tileLocation, eDirection direction) const
{
    Point2D directionVector = GetDirectionVector(direction);

    Point2D nextTilePosition = tileLocation + directionVector;
    return (nextTilePosition.x > -1) && (nextTilePosition.y > -1) && 
        (nextTilePosition.x < mDimensions.x) && 
        (nextTilePosition.y < mDimensions.y); 
}

void GameMap::FloodFill4(TilesArray& outputTiles, MapTile* origin, MapFloodFillFlags flags)
{
    Rect2D scanArea(0, 0, mDimensions.x, mDimensions.y);
    FloodFill4(outputTiles, origin, scanArea, flags);
}

void GameMap::FloodFill4(TilesArray& outputTiles, MapTile* origin, const Rect2D& scanArea, MapFloodFillFlags flags)
{
    // check conditions
    if (origin == nullptr || scanArea.mSizeX < 1 || scanArea.mSizeY < 1)
    {
        debug_assert(false);
        return;
    }

    outputTiles.clear();
    outputTiles.reserve(25);

    if (++mFloodFillCounter == 0)
    {
        ++mFloodFillCounter;
        ClearFloodFillCounter();
    }

    // explore tiles
    std::stack<MapTile*> explorationList;
    explorationList.push(origin); 

    while (!explorationList.empty())
    {
        MapTile* currentTile = explorationList.top();
        explorationList.pop();

        // already explored
        if (currentTile->mFloodFillCounter == mFloodFillCounter)
            continue;

        // move tile to cleselist
        currentTile->mFloodFillCounter = mFloodFillCounter;
        MapTile* tilesToExplore[] = 
        {
            currentTile->mNeighbours[eDirection_E],
            currentTile->mNeighbours[eDirection_N],
            currentTile->mNeighbours[eDirection_W],
            currentTile->mNeighbours[eDirection_S],
        };

        for (MapTile* tile: tilesToExplore)
        {
            if (!tile || tile->mFloodFillCounter == mFloodFillCounter) 
            {
                continue;
            }

            // bounds
            if (tile->mTileLocation.x < scanArea.mX) continue;
            if (tile->mTileLocation.y < scanArea.mY) continue;
            if (tile->mTileLocation.x >= scanArea.mX + scanArea.mSizeX) continue;
            if (tile->mTileLocation.y >= scanArea.mY + scanArea.mSizeY) continue;

            bool acceptableTile = flags.mSameBaseTerrain ? 
               (origin->GetBaseTerrain() == tile->GetBaseTerrain()) :
               (origin->GetTerrain() == tile->GetTerrain());

            if (acceptableTile && flags.mSameOwner)
            {
                TerrainDefinition* terrainDefinition = flags.mSameBaseTerrain ? tile->GetBaseTerrain() : tile->GetTerrain();
                if (terrainDefinition->mIsOwnable)
                {
                    acceptableTile = tile->mOwnerId == origin->mOwnerId;
                }
            }

            if (acceptableTile)
            {
                explorationList.push(tile);
            }
        }

        outputTiles.push_back(currentTile);
    }
}

void GameMap::ClearFloodFillCounter()
{
    for (MapTile& currTile: mTilesArray)
    {
        currTile.mFloodFillCounter = 0;
    }
}
