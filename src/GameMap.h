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
        TilesIterator(MapTile* initialTile, const Rect2D& mapArea);
        MapTile* NextTile();
        void Restart();
    public:
        MapTile* mInitialTile = nullptr;
        MapTile* mFromRowTile = nullptr;
        MapTile* mCurrentTile = nullptr;
        Rect2D mMapArea;
    };

    //////////////////////////////////////////////////////////////////////////

public:
    void LoadScenario(const ScenarioDefinition& scenarioData);
    void Cleanup();

    // iterate over map tiles within specified rectangular area
    TilesIterator IterateTiles() const;
    TilesIterator IterateTiles(const Rect2D& mapArea) const;
    TilesIterator IterateTiles(const Point2D& startTile, const Point2D& areaSize) const;

    // @param coord: World coordinates, y is ignored
    inline MapTile* GetTileAtPosition(const glm::vec3& coord) const
    {
        Point2D tileLocation = MapUtils::ComputeTileFromPosition(coord);
        return GetMapTileOrNull(tileLocation);
    }

    // @param coord: World coordinates 
    inline MapTile* GetTileAtPosition(const glm::vec2& coord) const
    {
        Point2D tileLocation = MapUtils::ComputeTileFromPosition(coord);
        return GetMapTileOrNull(tileLocation);
    }

    // Get map tile located at coordinates
    inline MapTile* GetMapTile(const Point2D& tileLocation) const
    {
        cxx_assert(WithinMap(tileLocation));
        return &mTiles[tileLocation.y * mDimensions.x + tileLocation.x];
    }

    inline MapTile* GetMapTileOrNull(const Point2D& tileLocation) const
    {
        return WithinMap(tileLocation) ? GetMapTile(tileLocation) : nullptr;
    }

    // Test whether tile is within map
    inline bool WithinMap(const Point2D& tileLocation) const 
    {
        return (tileLocation.x > -1) && (tileLocation.y > -1) && 
            (tileLocation.x < mDimensions.x) && 
            (tileLocation.y < mDimensions.y); 
    }

    inline const Point2D& GetDimensions() const { return mDimensions; }

    // Flood fill adjacent tiles in 4 directions with same player and terrain id
    void FloodFill4(cxx::any_vector<MapTile*> outTiles, MapTile* tileOrigin, unsigned int flags = FLOOD_FILL4_SAME_OWNER);
    void FloodFill4(cxx::any_vector<MapTile*> outTiles, MapTile* tileOrigin, Rect2D scanArea, unsigned int flags = FLOOD_FILL4_SAME_OWNER);

    // compute floor height at location
    float GetFloorHeightAt(const glm::vec2& coordinate) const;
    float GetFloorHeightAt(const glm::vec3& coordinate) const
    {
        return GetFloorHeightAt(glm::vec2 {coordinate.x, coordinate.z});
    }

private:
    // Internal get map tile and reset it to default state
    MapTile* GetTileInitialize(int tilex, int tiley, unsigned int randomValue) const;

    void InitTilesFloodFillCounter();

public:
    std::unique_ptr<MapTile[]> mTiles;
    Point2D mDimensions {0, 0};
    unsigned int mFloodFillCounter = 1;
    cxx::aabbox mBoundingBox {};

    std::vector<MapTile*> mFloodFillOpenListBuffer;
    std::vector<MapTile*> mFloodFillResultBuffer;
};

//////////////////////////////////////////////////////////////////////////

extern GameMap gGameMap;

//////////////////////////////////////////////////////////////////////////