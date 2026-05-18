#pragma once

#include "MapTile.h"
#include "MapUtils.h"

//////////////////////////////////////////////////////////////////////////

struct ScenarioDefinition;

//////////////////////////////////////////////////////////////////////////

// Flood fill flags
enum 
{
    FLOOD_FILL4_SAME_OWNER = (1 << 0), // match player id
    FLOOD_FILL4_SAME_BASE_TERRAIN = (1 << 1), // ignore overriden terrain type
};

//////////////////////////////////////////////////////////////////////////
// Level Map
//////////////////////////////////////////////////////////////////////////

class GameMap: public cxx::noncopyable
{
public:

    //////////////////////////////////////////////////////////////////////////

    // iterate over map tiles within specified rectangular area
    struct TilesIterator
    {
    public:
        TilesIterator(MapTile* initialTile, const MapArea2D& mapArea);
        MapTile* NextTile();
        void Restart();
    public:
        MapTile* mInitialTile = nullptr;
        MapTile* mFromRowTile = nullptr;
        MapTile* mCurrentTile = nullptr;
        MapArea2D mMapArea;
    };

    //////////////////////////////////////////////////////////////////////////

public:
    void LoadScenario(const ScenarioDefinition& scenarioData);
    void Cleanup();

    // iterate over map tiles within specified rectangular area
    TilesIterator IterateTiles() const;
    TilesIterator IterateTiles(const MapArea2D& mapArea) const;
    TilesIterator IterateTiles(const MapPoint2D& startTile, const MapPoint2D& areaSize) const;

    // @param coord: World coordinates, y is ignored
    inline MapTile* GetTileAtPosition(const glm::vec3& coord) const
    {
        MapPoint2D tileLocation = MapUtils::ComputeTileFromPosition(coord);
        // test tile coord is within map
        MapTile* resultTile = nullptr;
        if (WithinMap(tileLocation))
        {
            resultTile = GetMapTile(tileLocation);
        }
        return resultTile;
    }

    // @param coord: World coordinates 
    inline MapTile* GetTileAtPosition(const glm::vec2& coord) const
    {
        MapPoint2D tileLocation = MapUtils::ComputeTileFromPosition(coord);
        // test tile coord is within map
        MapTile* resultTile = nullptr;
        if (WithinMap(tileLocation))
        {
            resultTile = GetMapTile(tileLocation);
        }
        return resultTile;
    }

    // Get map tile located at coordinates
    inline MapTile* GetMapTile(const MapPoint2D& tileLocation) const
    {
        cxx_assert(WithinMap(tileLocation));
        return &mTiles[tileLocation.y * mDimensions.x + tileLocation.x];
    }

    // Test whether tile is within map
    inline bool WithinMap(const MapPoint2D& tileLocation) const 
    {
        return (tileLocation.x > -1) && (tileLocation.y > -1) && 
            (tileLocation.x < mDimensions.x) && 
            (tileLocation.y < mDimensions.y); 
    }

    inline const MapPoint2D& GetDimensions() const { return mDimensions; }

    // Flood fill adjacent tiles in 4 directions with same player and terrain id
    template<typename TContainer>
    inline void FloodFill4(TContainer& resultContainer, MapTile* tileOrigin, unsigned int flags = FLOOD_FILL4_SAME_OWNER)
    {
        MapArea2D mapArea { 0, 0, mDimensions.x, mDimensions.y };
        FloodFill4Impl(tileOrigin, mapArea, flags);
        resultContainer.assign(mFloodFillResultBuffer.begin(), mFloodFillResultBuffer.end());
    }
    template<typename TContainer>
    inline void FloodFill4(TContainer& resultContainer, MapTile* tileOrigin, MapArea2D scanArea, unsigned int flags = FLOOD_FILL4_SAME_OWNER)
    {
        FloodFill4Impl(tileOrigin, scanArea, flags);
        resultContainer.assign(mFloodFillResultBuffer.begin(), mFloodFillResultBuffer.end());
    }

    // compute floor height at location
    float GetFloorHeightAt(const glm::vec2& coordinate) const;
    float GetFloorHeightAt(const glm::vec3& coordinate) const
    {
        return GetFloorHeightAt(glm::vec2 {coordinate.x, coordinate.z});
    }

private:
    // Internal get map tile and reset it to default state
    MapTile* GetTileInitialize(int tilex, int tiley, unsigned int randomValue) const;

    void FloodFill4Impl(MapTile* tileOrigin, MapArea2D scanArea, unsigned int floodFillFlags = FLOOD_FILL4_SAME_OWNER);

    void InitTilesFloodFillCounter();

public:
    std::unique_ptr<MapTile[]> mTiles;
    MapPoint2D mDimensions {0, 0};
    unsigned int mFloodFillCounter = 1;
    cxx::aabbox mBoundingBox {};

    std::vector<MapTile*> mFloodFillOpenListBuffer;
    std::vector<MapTile*> mFloodFillResultBuffer;
};