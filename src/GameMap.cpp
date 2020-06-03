#include "pch.h"
#include "GameMap.h"
#include "randomizer.h"
#include <stack>

void GameMap::Setup(const Point& mapDimensions, unsigned int randomSeed)
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
            
        TerrainTile& currentTile = mTilesArray[tileIndex];
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

    for (TerrainTile& currTile: mTilesArray)
    {
        currTile.mRandomValue = randomize.generate_int();
    }

    ComputeBounds();
}

void GameMap::Clear()
{
    mFloodFillCounter = 0;
    mDimensions.x = 0;
    mDimensions.y = 0;
    mTilesArray.clear();
    mBounds.clear();
}

TerrainTile* GameMap::GetTileFromCoord3d(const glm::vec3& coord)
{
    Point tileLocation {
        static_cast<int>((coord.x + DUNGEON_CELL_HALF_SIZE) / DUNGEON_CELL_SIZE),
        static_cast<int>((coord.z + DUNGEON_CELL_HALF_SIZE) / DUNGEON_CELL_SIZE)
    };
    return GetMapTile(tileLocation);
}

TerrainTile* GameMap::GetMapTile(const Point& tileLocation)
{
    if (IsWithinMap(tileLocation))
        return &mTilesArray[tileLocation.y * mDimensions.x + tileLocation.x];

    return nullptr;
}

TerrainTile* GameMap::GetMapTile(const Point& tileLocation, eDirection direction)
{
    Point directionVector = GetDirectionVector(direction);

    return GetMapTile(tileLocation + directionVector);
}

bool GameMap::IsWithinMap(const Point& tileLocation, eDirection direction) const
{
    Point directionVector = GetDirectionVector(direction);

    Point nextTilePosition = tileLocation + directionVector;
    return (nextTilePosition.x > -1) && (nextTilePosition.y > -1) && 
        (nextTilePosition.x < mDimensions.x) && 
        (nextTilePosition.y < mDimensions.y); 
}

void GameMap::FloodFill4(TilesList& outputTiles, TerrainTile* origin, MapFloodFillFlags flags)
{
    Rectangle scanArea(0, 0, mDimensions.x, mDimensions.y);
    FloodFill4(outputTiles, origin, scanArea, flags);
}

void GameMap::FloodFill4(TilesList& outputTiles, TerrainTile* origin, const Rectangle& scanArea, MapFloodFillFlags flags)
{
    // check conditions
    if (origin == nullptr || scanArea.w < 1 || scanArea.h < 1)
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
    std::stack<TerrainTile*> explorationList;
    explorationList.push(origin); 

    while (!explorationList.empty())
    {
        TerrainTile* currentTile = explorationList.top();
        explorationList.pop();

        // already explored
        if (currentTile->mFloodFillCounter == mFloodFillCounter)
            continue;

        // move tile to cleselist
        currentTile->mFloodFillCounter = mFloodFillCounter;
        TerrainTile* tilesToExplore[] = 
        {
            currentTile->mNeighbours[eDirection_E],
            currentTile->mNeighbours[eDirection_N],
            currentTile->mNeighbours[eDirection_W],
            currentTile->mNeighbours[eDirection_S],
        };

        for (TerrainTile* tile: tilesToExplore)
        {
            if (!tile || tile->mFloodFillCounter == mFloodFillCounter) 
            {
                continue;
            }

            // bounds
            if (tile->mTileLocation.x < scanArea.x) continue;
            if (tile->mTileLocation.y < scanArea.y) continue;
            if (tile->mTileLocation.x >= scanArea.x + scanArea.w) continue;
            if (tile->mTileLocation.y >= scanArea.y + scanArea.h) continue;

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

void GameMap::ComputeBounds()
{
    cxx::aabbox minPosBounds;
    cxx::aabbox maxPosBounds;
    GetMapBlockBounds(Point(0, 0), minPosBounds);
    GetMapBlockBounds(mDimensions - Point(1, 1), maxPosBounds);
    mBounds = minPosBounds.union_with(maxPosBounds);
}

void GameMap::ClearFloodFillCounter()
{
    for (TerrainTile& currTile: mTilesArray)
    {
        currTile.mFloodFillCounter = 0;
    }
}
