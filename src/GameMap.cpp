#include "stdafx.h"
#include "GameMap.h"
#include "GameRenderManager.h"
#include "ScenarioDefs.h"
#include "GameMain.h"
#include "GameWorld.h"

//////////////////////////////////////////////////////////////////////////

enum 
{
    FLOOD_FILL_BUFFERS_RESERVATION_SIZE = 1024
};

//////////////////////////////////////////////////////////////////////////

GameMap gGameMap;

//////////////////////////////////////////////////////////////////////////

GameMap::TilesIterator::TilesIterator(MapTile* initialTile, const Rect2D& mapArea)
    : mInitialTile(initialTile)
    , mCurrentTile(initialTile)
    , mFromRowTile(initialTile)
    , mMapArea(mapArea)
{
}

MapTile* GameMap::TilesIterator::NextTile()
{
    cxx_assert(mInitialTile);
    MapTile* resultTile = mCurrentTile;

    // advance
    if (mCurrentTile)
    {
        // proceed to the next tile on current row
        mCurrentTile = mCurrentTile->mNeighbours[eDirection_E];
        if (mCurrentTile && mCurrentTile->mLocation.x < (mMapArea.x + mMapArea.w))
        {
            // done
        }
        else // proceed to the next row
        {
            mFromRowTile = mFromRowTile->mNeighbours[eDirection_S];
            if (mFromRowTile && mFromRowTile->mLocation.y < (mMapArea.y + mMapArea.h))
            {
                mCurrentTile = mFromRowTile;
            }
            else
            {
                mCurrentTile = nullptr;
            }
        }
    }

    return resultTile;
}

void GameMap::TilesIterator::Restart()
{
    mCurrentTile = mInitialTile;
    mFromRowTile = mInitialTile;
}

//////////////////////////////////////////////////////////////////////////

void GameMap::LoadScenario(const ScenarioDefinition& scenarioData)
{
    cxx_assert(mTiles == nullptr);
    mDimensions = { scenarioData.mLevelInfo.mMapDimsX, scenarioData.mLevelInfo.mMapDimsY };

    cxx_assert(mDimensions.x <= MAX_DUNGEON_MAP_DIMENSIONS);
    cxx_assert(mDimensions.y <= MAX_DUNGEON_MAP_DIMENSIONS);

    // allocate map tiles matrix
    mTiles = std::move(std::unique_ptr<MapTile[]>(new MapTile[mDimensions.x * mDimensions.y]));

    // initialize tiles
    for (int tiley = 0; tiley < mDimensions.y; ++tiley)
    {
        for (int tilex = 0; tilex < mDimensions.x; ++tilex)
        {
            // use random number generator for tile visuals diversity
            unsigned int randomValue = Random::GenerateUint();

            MapTile* currentTile = GetTileInitialize(tilex, tiley, randomValue);

            // setup terrain definition
            const int tileIndex = (tiley * mDimensions.x) + tilex;

            // terrain types
            TerrainDefinition* tileBaseTerrain = nullptr;
            TerrainDefinition* tileRoomTerrain = nullptr;

            // acquire terrain type
            TerrainTypeId tileTerrainType = scenarioData.mMapTiles[tileIndex].mTerrainType;
            if (scenarioData.IsRoomTypeTerrain(tileTerrainType))
            {
                RoomDefinition* roomDefinition = scenarioData.GetRoomDefinitionByTerrain(tileTerrainType);

                // acquire terrain type under the bridge
                if (!roomDefinition->mPlaceableOnLand)
                {
                    switch (scenarioData.mMapTiles[tileIndex].mTerrainUnderTheBridge)
                    {
                        case eBridgeTerrain_Lava:
                            tileBaseTerrain = scenarioData.GetTerrainDefinition(scenarioData.mLavaTerrainType);
                        break;
                        case eBridgeTerrain_Water:
                            tileBaseTerrain = scenarioData.GetTerrainDefinition(scenarioData.mWaterTerrainType);
                        break;
                        default: // should be an error
                        {
                            cxx_assert(false);
                        }
                        break;
                    }
                }
                else
                {
                    // claimed path is default
                    tileBaseTerrain = scenarioData.GetTerrainDefinition(scenarioData.mPlayerColouredPathTerrainType);
                }
                // override terrain type
                tileRoomTerrain = scenarioData.GetTerrainDefinition(tileTerrainType);
            }
            else
            {
                tileBaseTerrain = scenarioData.GetTerrainDefinition(tileTerrainType);
            }

            // set tile info
            currentTile->SetBaseTerrain(tileBaseTerrain);
            currentTile->SetRoomTerrain(tileRoomTerrain);
            currentTile->mOwnerId = scenarioData.mMapTiles[tileIndex].mOwnerId;

            // setup neighbours
            if (tiley > 0)
            {
                if (tilex > 0) 
                {
                    currentTile->mNeighbours[eDirection_NW] = GetMapTile({tilex - 1, tiley - 1});
                }

                if (tilex < mDimensions.x - 1) 
                {
                    currentTile->mNeighbours[eDirection_NE] = GetMapTile({tilex + 1, tiley - 1});
                }
                currentTile->mNeighbours[eDirection_N] = GetMapTile({tilex, tiley - 1});
            }
            if (tilex > 0) 
            {
                currentTile->mNeighbours[eDirection_W] = GetMapTile({tilex - 1, tiley});
            }

            if (tilex < mDimensions.x - 1) 
            {
                currentTile->mNeighbours[eDirection_E] = GetMapTile({tilex + 1, tiley});
            }

            if (tiley < mDimensions.y - 1)
            {
                if (tilex > 0) 
                {
                    currentTile->mNeighbours[eDirection_SW] = GetMapTile({tilex - 1, tiley + 1});
                }

                if (tilex < mDimensions.x - 1) 
                {
                    currentTile->mNeighbours[eDirection_SE] = GetMapTile({tilex + 1, tiley + 1});
                }
                currentTile->mNeighbours[eDirection_S] = GetMapTile({tilex, tiley + 1});
            }
        }
    }
    // update bounding box
    mBoundingBox.mMin = { -MAP_TILE_SIZE * 0.5f - 0.01f, 1.0f - 0.01f, -MAP_TILE_SIZE * 0.5f - 0.01f };
    mBoundingBox.mMax = {
        mDimensions.x * MAP_TILE_SIZE - (MAP_TILE_SIZE * 0.5f) + 0.01f,
        MAP_TILE_SIZE + 1.01f,
        mDimensions.y * MAP_TILE_SIZE - (MAP_TILE_SIZE * 0.5f) + 0.01f
    };

    // warmup buffers
    mFloodFillOpenListBuffer.reserve(FLOOD_FILL_BUFFERS_RESERVATION_SIZE);
    mFloodFillResultBuffer.reserve(FLOOD_FILL_BUFFERS_RESERVATION_SIZE);
}

void GameMap::Cleanup()
{
    mTiles.reset();
    mDimensions = {};
    mFloodFillCounter = 1;
}

GameMap::TilesIterator GameMap::IterateTiles(const Rect2D& mapArea) const
{
    const Rect2D fullMapArea {0, 0, mDimensions.x, mDimensions.y};
    const Rect2D insersetionArea = fullMapArea.GetIntersection(mapArea);
    if (insersetionArea.Empty())
    {
        return TilesIterator(nullptr, {});
    }
    const Point2D initialTileLocation (insersetionArea.x, insersetionArea.y);
    MapTile* initialTile = GetMapTile(initialTileLocation);
    return TilesIterator(initialTile, insersetionArea);
}

GameMap::TilesIterator GameMap::IterateTiles(const Point2D& startTile, const Point2D& areaSize) const
{
    Rect2D rc ( startTile.x, startTile.y, areaSize.x, areaSize.y );
    return IterateTiles(rc);
}

GameMap::TilesIterator GameMap::IterateTiles() const
{
    Rect2D rc ( 0, 0, mDimensions.x, mDimensions.y );
    return IterateTiles(rc);
}

float GameMap::GetFloorHeightAt(const glm::vec2& coordinate) const
{
    static const float inv_TileSize = 1.0f / MAP_TILE_SIZE;
    if (const MapTile* mapTile = GetTileAtPosition(coordinate))
    {
        const glm::vec2 coordWithinTile = MapUtils::WrapPositionWithinBlock(coordinate);  

        // normalized coords
        float nc_x = (coordWithinTile.x * inv_TileSize);
        float nc_y = (coordWithinTile.y * inv_TileSize);

        int sampleX = std::clamp(static_cast<int>(nc_x * TileHeightmap::Resolution), 0, TileHeightmap::Resolution - 1);
        int sampleY = std::clamp(static_cast<int>(nc_y * TileHeightmap::Resolution), 0, TileHeightmap::Resolution - 1);
        return mapTile->mFloorHeightmap.mSamples[sampleY * TileHeightmap::Resolution + sampleX];
    }

    return MAP_FLOOR_LEVEL;
}

MapTile* GameMap::GetTileInitialize(int tilex, int tiley, unsigned int randomValue) const
{
    MapTile* currentTile = &mTiles[tiley * mDimensions.x + tilex];

    currentTile->mLocation.x = tilex;
    currentTile->mLocation.y = tiley;
    currentTile->mTaggedByPlayers.Clear();
    currentTile->mIsRoomInnerTile = false;
    currentTile->mIsRoomEntrance = false;
    currentTile->mRandomValue = randomValue;
    currentTile->mHitPoints = 100;
    currentTile->mFloodFillCounter = 0;
        
    currentTile->ClearFloorHeightmap();
    currentTile->ClearAreaCode();
    return currentTile;
}

void GameMap::FloodFill4(cxx::any_vector<MapTile*> outTiles, MapTile* tileOrigin, unsigned int flags)
{
    const Rect2D mapArea { 0, 0, mDimensions.x, mDimensions.y };
    FloodFill4(outTiles, tileOrigin, mapArea, flags);
}

void GameMap::FloodFill4(cxx::any_vector<MapTile*> outTiles, MapTile* tileOrigin, Rect2D scanArea, unsigned int flags)
{
    mFloodFillResultBuffer.clear();
    mFloodFillResultBuffer.reserve(32);

    // explore tiles
    mFloodFillOpenListBuffer.clear();
    mFloodFillOpenListBuffer.push_back(tileOrigin); 
    while (!mFloodFillOpenListBuffer.empty())
    {
        MapTile* currentTile = mFloodFillOpenListBuffer.back();
        mFloodFillOpenListBuffer.pop_back();

        // already explored
        if (currentTile->mFloodFillCounter == mFloodFillCounter)
            continue;

        // move tile to close list
        currentTile->mFloodFillCounter = mFloodFillCounter;
        MapTile* tilesToExplore[] = 
        {
            currentTile->mNeighbours[eDirection_E],
            currentTile->mNeighbours[eDirection_N],
            currentTile->mNeighbours[eDirection_W],
            currentTile->mNeighbours[eDirection_S]
        };

        for (MapTile* tile: tilesToExplore)
        {
            if (!tile || tile->mFloodFillCounter == mFloodFillCounter) 
            {
                continue;
            }

            // bounds
            if (tile->mLocation.x < scanArea.x) continue;
            if (tile->mLocation.y < scanArea.y) continue;
            if (tile->mLocation.x >= (scanArea.x + scanArea.w)) continue;
            if (tile->mLocation.y >= (scanArea.y + scanArea.h)) continue;

            bool sameTerrain = (flags & FLOOD_FILL4_SAME_BASE_TERRAIN) ? 
                tileOrigin->SameTileBaseTerrainType(tile) : 
                tileOrigin->SameTileTerrainType(tile);

            if (sameTerrain)
            {
                if (flags & FLOOD_FILL4_SAME_OWNER)
                {
                    const TerrainDefinition* terrainDefinition = (flags & FLOOD_FILL4_SAME_BASE_TERRAIN) ?
                        tile->GetBaseTerrain() : 
                        tile->GetTerrain();

                    if (terrainDefinition->mIsOwnable)
                    {
                        sameTerrain = (tile->mOwnerId == tileOrigin->mOwnerId);
                    }
                }
            }

            if (sameTerrain)
            {
                mFloodFillOpenListBuffer.push_back(tile);
            }
        }

        mFloodFillResultBuffer.push_back(currentTile);
    }

    // update flood fill counter
    if (++mFloodFillCounter == 0)
    {
        ++mFloodFillCounter;
        InitTilesFloodFillCounter();
    }

    if (!mFloodFillResultBuffer.empty())
    {
        outTiles.assign(mFloodFillResultBuffer);
        mFloodFillResultBuffer.clear();
    }
}

void GameMap::InitTilesFloodFillCounter()
{
    const int numTiles = mDimensions.x * mDimensions.y;
    for (int itile = 0; itile < numTiles; ++itile)
    {
        MapTile& roller = mTiles[itile];
        roller.mFloodFillCounter = 0;
    }
}
