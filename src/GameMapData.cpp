#include "pch.h"
#include "GameMapData.h"
#include "randomizer.h"
#include <stack>

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

void GameMapData::FloodFill4(GameMapTiles& outputTiles, GameMapTile* origin, MapFloodFillFlags flags)
{
    Rect2D scanArea(0, 0, mDimensions.x, mDimensions.y);
    FloodFill4(outputTiles, origin, scanArea, flags);
}

void GameMapData::FloodFill4(GameMapTiles& outputTiles, GameMapTile* origin, const Rect2D& scanArea, MapFloodFillFlags flags)
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
    std::stack<GameMapTile*> explorationList;
    explorationList.push(origin); 

    while (!explorationList.empty())
    {
        GameMapTile* currentTile = explorationList.top();
        explorationList.pop();

        // already explored
        if (currentTile->mFloodFillCounter == mFloodFillCounter)
            continue;

        // move tile to cleselist
        currentTile->mFloodFillCounter = mFloodFillCounter;
        GameMapTile* tilesToExplore[] = 
        {
            currentTile->mNeighbours[eDirection_E],
            currentTile->mNeighbours[eDirection_N],
            currentTile->mNeighbours[eDirection_W],
            currentTile->mNeighbours[eDirection_S],
        };

        for (GameMapTile* tile: tilesToExplore)
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

void GameMapData::ClearFloodFillCounter()
{
    for (GameMapTile& currTile: mTilesArray)
    {
        currTile.mFloodFillCounter = 0;
    }
}
