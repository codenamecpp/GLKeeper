#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameMapDefs.h"
#include "PathFindRequest.h"

//////////////////////////////////////////////////////////////////////////

class NavigationService final: public cxx::noncopyable
{
public:
    NavigationService();
    ~NavigationService();

    void EnterWorld();
    void ClearWorld();
    void UpdateFrame(float deltaTime);
    void UpdateLogic(float stepDeltaTime);

    // fast check whether a path exists between two tiles for a specific passability type
    // it uses the map area code to determine whether both tiles are connected
    bool CheckPathExists(const Point2D& srcCoord, const Point2D& dstCoord, ePassabilityType passabilityType) const;
    bool CheckPathExists(const glm::vec2& srcPosition, const glm::vec2& dstPosition, ePassabilityType passabilityType) const;

    bool GetRandomWanderingPoint(const glm::vec2& srcPosition, ePassabilityType passabilityType,
        int minTilesDistance, 
        int maxTilesDistance, glm::vec2& resultPoint);

    bool GetRandomPointWithinTile(const Point2D& mapTile, glm::vec2& resultPoint);

    void UpdateAreaCodes(cxx::span<MapTile*> mapTiles);

    // queues a pathfinding request, actual pathfinding will start after previous requests are processed
    // returns null if params are invalid
    PathFindRequestPtr RequestPath(const glm::vec2& srcPosition, const glm::vec2& dstPosition, ePassabilityType passabilityType);

private:
    // returns new area code
    MapAreaCode NextAreaCode();
    MapAreaCode FloodFillNewArea(MapTile* startTile, ePassabilityType passabilityType);
    MapAreaCode FloodFillArea(MapTile* startTile, ePassabilityType passabilityType);

    void InitMapAreaCodes();
    void InitMapAreaCodes(ePassabilityType passabilityType);
    void UpdateMapAreaCodes(MapTile* startTile, ePassabilityType passabilityType, MapAreaCode minNewAreaCode);

    static PathFindRequestPtr NewPathFindRequest();

    void RemovePathFindRequest(PathFindRequest* request);
    void LaunchPathFindRequest(PathFindRequest* request);
    void ProcessPathFindRequests();

private:
    MapAreaCode mNextAreaCode = 1;

    std::vector<MapTile*> mFloodFillOpenListBuffer;
    std::vector<PathFindRequest*> mPathFindRequests;

    std::unique_ptr<PathFindEngine> mPathFindEngine;
};

//////////////////////////////////////////////////////////////////////////

extern NavigationService gNavigationService;

//////////////////////////////////////////////////////////////////////////