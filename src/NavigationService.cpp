#include "stdafx.h"
#include "NavigationService.h"
#include "GameWorld.h"
#include "SimplePool.h"
#include "GameMain.h"
#include "PathFindUtils.h"
#include "PathFindEngine.h"
#include "GameMap.h"

//////////////////////////////////////////////////////////////////////////

NavigationService gNavigationService;

//////////////////////////////////////////////////////////////////////////

NavigationService::NavigationService()
{
    mPathFindEngine = std::make_unique<PathFindEngine>();

    mFloodFillOpenListBuffer.reserve(1024);
}

NavigationService::~NavigationService()
{
}

void NavigationService::EnterWorld()
{
    mNextAreaCode = 1;

    InitMapAreaCodes();

    mPathFindEngine->EnterWorld();
}

void NavigationService::ClearWorld()
{
    mPathFindEngine->ClearWorld();
}

void NavigationService::UpdateFrame(float deltaTime)
{
    ProcessPathFindRequests();
}

void NavigationService::UpdateLogic(float stepDeltaTime)
{
}

bool NavigationService::GetRandomWanderingPoint(const glm::vec2& srcPosition, ePassabilityType passabilityType,
    int minTilesDistance, 
    int maxTilesDistance, glm::vec2& resultPoint) 
{
    const MapPoint2D srcTileLocation = MapUtils::ComputeTileFromPosition(srcPosition);
    cxx_assert(maxTilesDistance >= minTilesDistance);
    if ((minTilesDistance < 1) || !gGameMap.WithinMap(srcTileLocation))
    {
        cxx_assert(false);
        return false;
    }
    
    maxTilesDistance = std::max(minTilesDistance, maxTilesDistance);

    cxx_assert(mFloodFillOpenListBuffer.empty());

    MapTile* srcTile = gGameMap.GetMapTile(srcTileLocation);
    cxx_assert(srcTile);

    // collect all reachable tiles around
    GameMap::TilesIterator tilesIterator = gGameMap.IterateTiles(
        MapArea2D { srcTileLocation.x - maxTilesDistance, srcTileLocation.y - maxTilesDistance, 
            maxTilesDistance * 2 + 1, 
            maxTilesDistance * 2 + 1 });
    for (MapTile* mapTile = tilesIterator.NextTile(); mapTile; 
        mapTile = tilesIterator.NextTile())
    {
        MapPoint2D tileDist = mapTile->mLocation - srcTileLocation;
        if ((abs(tileDist.x) < minTilesDistance) &&
            (abs(tileDist.y) < minTilesDistance))
        {
            continue;
        }
        if (srcTile->GetAreaCode(passabilityType) == 
            mapTile->GetAreaCode(passabilityType))
        {
            mFloodFillOpenListBuffer.push_back(mapTile);
        }
    }

    if (mFloodFillOpenListBuffer.empty())
    {
        return false;
    }

    const unsigned int randomTileIndex = Random::GenerateUint(0, mFloodFillOpenListBuffer.size() - 1);

    MapTile* destinationTile = mFloodFillOpenListBuffer[randomTileIndex];
    mFloodFillOpenListBuffer.clear();

    if (!GetRandomPointWithinTile(destinationTile->mLocation, resultPoint))
    {
        cxx_assert(false);
        return false;
    }

    return true;
}

void NavigationService::InitMapAreaCodes()
{
    // reset area codes
    GameMap::TilesIterator tilesIterator = gGameMap.IterateTiles();
    for (MapTile* mapTile = tilesIterator.NextTile(); mapTile; 
        mapTile = tilesIterator.NextTile())
    {
        mapTile->ClearAreaCode();
    }

    for (int passability = 0; passability < ePassabilityType_COUNT; ++passability)
    {
        InitMapAreaCodes(static_cast<ePassabilityType>(passability));
    }
}

void NavigationService::InitMapAreaCodes(ePassabilityType passabilityType)
{
    cxx_assert(passabilityType < ePassabilityType_COUNT);

    GameMap::TilesIterator tilesIterator = gGameMap.IterateTiles();
    for (MapTile* mapTile = tilesIterator.NextTile(); mapTile; 
        mapTile = tilesIterator.NextTile())
    {
        FloodFillNewArea(mapTile, passabilityType);
    }
}

MapAreaCode NavigationService::NextAreaCode()
{
    MapAreaCode resultCode = mNextAreaCode++;
    cxx_assert(resultCode > 0);
    return resultCode;
}

MapAreaCode NavigationService::FloodFillNewArea(MapTile* startTile, ePassabilityType passabilityType)
{
    cxx_assert(startTile);
    cxx_assert(passabilityType < ePassabilityType_COUNT);
    cxx_assert(mFloodFillOpenListBuffer.empty());

    if (startTile == nullptr) return 0;

    auto CheckTilePassable = [passabilityType](const MapTile* mapTile) -> bool
        {
            if (mapTile == nullptr) return false;
            // start tile area must not be initialized at this point
            if (mapTile->GetAreaCode(passabilityType) > 0)
                return false;
            return PathFindUtils::CheckPassability(mapTile, mapTile, passabilityType);
        };

    // verify can start flood filling area
    if (!CheckTilePassable(startTile))
        return 0;

    MapAreaCode newAreaCode = NextAreaCode();

    // set initial tile area code
    startTile->mAreaCode[passabilityType] = newAreaCode;
    for (eDirection dir: gStraightDirections)
    {
        mFloodFillOpenListBuffer.push_back(startTile->mNeighbours[dir]);
    }

    // process surrounding tiles
    while (!mFloodFillOpenListBuffer.empty())
    {
        MapTile* checkTile = mFloodFillOpenListBuffer.back();
        mFloodFillOpenListBuffer.pop_back();
        if (!CheckTilePassable(checkTile))
            continue;

        checkTile->mAreaCode[passabilityType] = newAreaCode;
        for (eDirection dir: gStraightDirections)
        {
            MapTile* neighbourTile = checkTile->mNeighbours[dir];
            if (neighbourTile && (neighbourTile->GetAreaCode(passabilityType) == 0))
            {
                mFloodFillOpenListBuffer.push_back(neighbourTile);
            }
        }
    }
    cxx_assert(mFloodFillOpenListBuffer.empty());
    return newAreaCode;
}

MapAreaCode NavigationService::FloodFillArea(MapTile* startTile, ePassabilityType passabilityType)
{
    cxx_assert(startTile);
    cxx_assert(passabilityType < ePassabilityType_COUNT);
    cxx_assert(mFloodFillOpenListBuffer.empty());

    // verify can start flood filling area
    if (!PathFindUtils::CheckPassability(startTile, startTile, passabilityType))
        return 0;

    MapAreaCode newAreaCode = NextAreaCode();

    // set initial tile area code
    startTile->mAreaCode[passabilityType] = newAreaCode;
    for (eDirection dir: gStraightDirections)
    {
        mFloodFillOpenListBuffer.push_back(startTile->mNeighbours[dir]);
    }

    // process surrounding tiles
    while (!mFloodFillOpenListBuffer.empty())
    {
        MapTile* checkTile = mFloodFillOpenListBuffer.back();
        mFloodFillOpenListBuffer.pop_back();

        if (!PathFindUtils::CheckPassability(startTile, checkTile, passabilityType))
            continue;

        checkTile->mAreaCode[passabilityType] = newAreaCode;
        for (eDirection dir: gStraightDirections)
        {
            MapTile* neighbourTile = checkTile->mNeighbours[dir];
            if ((neighbourTile == nullptr) || 
                (neighbourTile->GetAreaCode(passabilityType) >= newAreaCode))
            {
                continue;
            }
            mFloodFillOpenListBuffer.push_back(neighbourTile);
        }
    }
    cxx_assert(mFloodFillOpenListBuffer.empty());
    return newAreaCode;
}

void NavigationService::UpdateAreaCodes(cxx::span<MapTile*> mapTiles)
{
    if (mapTiles.empty()) 
        return;

    const MapAreaCode minNewAreaCode = mNextAreaCode;
    for (int passability = 0; passability < ePassabilityType_COUNT; ++passability)
    {
        for (MapTile* rollerTile: mapTiles)
        {
            UpdateMapAreaCodes(rollerTile, static_cast<ePassabilityType>(passability), minNewAreaCode);
        }
    }
}

void NavigationService::UpdateMapAreaCodes(MapTile* startTile, ePassabilityType passabilityType, MapAreaCode minNewAreaCode)
{
    cxx_assert(startTile);
    cxx_assert(passabilityType < ePassabilityType_COUNT);

    MapAreaCode prevAreaCode = startTile->GetAreaCode(passabilityType);
    if (prevAreaCode >= minNewAreaCode)
        return;
    
    bool prevPassable = (prevAreaCode > 0);
    bool currPassable = PathFindUtils::CheckPassability(startTile, startTile, passabilityType);
    if (prevPassable == currPassable)
        return;

    if (!currPassable) // made impassable
    {
        startTile->mAreaCode[passabilityType] = 0;
     
        for (eDirection dir: gStraightDirections)
        {
            MapTile* neighbourTile = startTile->mNeighbours[dir];
            if ((neighbourTile == nullptr) || 
                (neighbourTile->GetAreaCode(passabilityType) != prevAreaCode)) 
            {
                continue;
            }

            FloodFillArea(neighbourTile, passabilityType);
        }

    }
    else // made passable
    {

        // best case scenario: all neighbours have the same area code
        MapAreaCode commonAreaCode = 0;
        for (eDirection dir: gStraightDirections)
        {
            MapTile* neighbourTile = startTile->mNeighbours[dir];
            if (neighbourTile == nullptr)
                continue;

            const MapAreaCode neighbourAreaCode = neighbourTile->GetAreaCode(passabilityType);
            if (neighbourAreaCode == 0)
                continue;

            if (commonAreaCode == 0)
            {
                commonAreaCode = neighbourAreaCode;
                continue;
            }
            if (commonAreaCode != neighbourAreaCode)
            {
                commonAreaCode = 0;
                break;
            }
        }

        if (commonAreaCode != 0)
        {
            startTile->mAreaCode[passabilityType] = commonAreaCode;
        }
        else // areas connected, need flood fill
        {
            FloodFillArea(startTile, passabilityType);
        }
    }
}

bool NavigationService::CheckPathExists(const glm::vec2& srcPosition, const glm::vec2& dstPosition, ePassabilityType passabilityType) const
{
    const MapPoint2D srcTileCoord = MapUtils::ComputeTileFromPosition(srcPosition);
    const MapPoint2D dstTileCoord = MapUtils::ComputeTileFromPosition(dstPosition);
    return CheckPathExists(srcTileCoord, dstTileCoord, passabilityType);
}

bool NavigationService::CheckPathExists(const MapPoint2D& srcCoord, const MapPoint2D& dstCoord, ePassabilityType passabilityType) const
{
    if (srcCoord == dstCoord) return true;

    if (gGameMap.WithinMap(srcCoord) && gGameMap.WithinMap(dstCoord))
    {
        const MapTile* srcTile = gGameMap.GetMapTile(srcCoord);
        const MapTile* dstTile = gGameMap.GetMapTile(dstCoord);
        cxx_assert(srcTile != dstTile);
        return srcTile->GetAreaCode(passabilityType) == dstTile->GetAreaCode(passabilityType);
    }
    return false;
}

PathFindRequestPtr NavigationService::NewPathFindRequest()
{
    static SimplePool<PathFindRequest> objectsPool = (
        [](PathFindRequest* object)
        {
            object->OnRecycle();
        });

    PathFindRequest* instancePtr = objectsPool.Acquire();
    return std::move(PathFindRequestPtr (instancePtr, [](PathFindRequest* object)
        {
            if (object)
            {
                gNavigationService.RemovePathFindRequest(object);
                objectsPool.Return(object);
            }
        }));
}

void NavigationService::RemovePathFindRequest(PathFindRequest* request)
{
    cxx_assert(request);

    if (!mPathFindRequests.empty())
    {
        // stop processing request
        if ((mPathFindRequests.front() == request) && request->IsSearching())
        {
            mPathFindEngine->ClearPathFind();
        }
        cxx::erase(mPathFindRequests, request);
    }
}

void NavigationService::ProcessPathFindRequests()
{
    const unsigned int CyclesBudgetPerFrame = 800; // todo: tune this value

    if (mPathFindRequests.empty())
        return;

    unsigned int cyclesCount = CyclesBudgetPerFrame;
    do
    {
        PathFindRequest* currRequest = mPathFindRequests.front();

        // not started yet?
        if (currRequest->GetStatus() == ePathFindStatus_Pending)
        {
            LaunchPathFindRequest(currRequest);
        }

        while (currRequest->IsSearching())
        {
            unsigned int cyclesTaken = 0;
            if (!mPathFindEngine->RunPathFind(cyclesCount, cyclesTaken))
                break;

            cxx_assert(cyclesCount >= cyclesTaken);
            cyclesCount -= cyclesTaken;
            if (cyclesCount == 0)
                break;
        }

        if (!currRequest->IsCompleted())
        {
            currRequest->SetStatus(ePathFindStatus_Completed);
            currRequest->SetResultPathFromTiles(mPathFindEngine->GetResultPath());
            RemovePathFindRequest(currRequest);
        }

    } while ((cyclesCount > 0) && !mPathFindRequests.empty());
}

void NavigationService::LaunchPathFindRequest(PathFindRequest* request)
{
    cxx_assert(request);

    unsigned int cyclesLimit = 0; // no limits if target if path exists
    if (!CheckPathExists(request->GetSrcTile(), request->GetDstTile(), request->GetPassabilityType()))
    {
        cyclesLimit = 2400; // todo: tune this value
    }

    if (mPathFindEngine->StartPathFind(request->GetSrcTile(), request->GetDstTile(), request->GetPassabilityType(), cyclesLimit))
    {
        request->SetStatus(ePathFindStatus_Searching);
    }
    else
    {
        cxx_assert(false);
        request->SetStatus(ePathFindStatus_Completed);
    }
}

PathFindRequestPtr NavigationService::RequestPath(const glm::vec2& srcPosition, const glm::vec2& dstPosition, ePassabilityType passabilityType)
{
    cxx_assert(passabilityType < ePassabilityType_COUNT);

    PathFindRequestPtr request = NewPathFindRequest();
    request->Configure(srcPosition, dstPosition, passabilityType);
    request->SetStatus(ePathFindStatus_Pending);

    if (srcPosition == dstPosition)
    {
        request->SetStatus(ePathFindStatus_Completed);
    }
    else // add to pending requests
    {
        mPathFindRequests.push_back(request.get());
    }
    return std::move(request);
}

bool NavigationService::GetRandomPointWithinTile(const MapPoint2D& mapTile, glm::vec2& resultPoint)
{
    resultPoint = MapUtils::ComputeTileCenter2d(mapTile);

    // add random offset
    const float maxOffsetFromTileCenter = MAP_TILE_HALF_SIZE * 0.8f;
    const float minOffsetFromTileCenter = -maxOffsetFromTileCenter;
    resultPoint.x += Random::GenerateFloat(minOffsetFromTileCenter, maxOffsetFromTileCenter);
    resultPoint.y += Random::GenerateFloat(minOffsetFromTileCenter, maxOffsetFromTileCenter);

    return true;
}
