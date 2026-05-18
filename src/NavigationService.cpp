#include "stdafx.h"
#include "NavigationService.h"
#include "GameWorld.h"

void NavigationService::EnterWorld()
{

}

void NavigationService::ClearWorld()
{

}

void NavigationService::UpdateFrame(float deltaTime)
{

}

void NavigationService::UpdateLogic(float stepDeltaTime)
{

}

bool NavigationService::GetRandomWanderingPointLocation(const MapPoint2D& srcTileLocation, 
    int minTilesDistance, 
    int maxTilesDistance, glm::vec2& resultPoint) const
{
    cxx_assert(maxTilesDistance >= minTilesDistance);
    cxx_assert(minTilesDistance >= 1);

    GameMap& gameMap = GetGameWorld().GetGameMap();
    if (!gameMap.WithinMap(srcTileLocation))
    {
        cxx_assert(false);
        return false;
    }

    if (minTilesDistance < 1) return false;
    
    maxTilesDistance = std::max(minTilesDistance, maxTilesDistance);

    MapTile* sourceTile = gameMap.GetMapTile(srcTileLocation);
    cxx_assert(sourceTile);

    int bestDistance = 0;
    eDirection bestDistanceDirection {};

    for (eDirection dir: gStraightDirections)
    {
        MapTile* currentTile = sourceTile;
        for (int tileCounter = 0;;)
        {
            MapTile* nextTile = nullptr;
            if (tileCounter < maxTilesDistance)
            {
                nextTile = currentTile->mNeighbours[dir];
            }

            if ((nextTile == nullptr) || nextTile->IsTerrainSolid())
            {
                if (bestDistance < tileCounter)
                {
                    bestDistance = tileCounter;
                    bestDistanceDirection = dir;
                }
                break;
            }
            currentTile = nextTile;
            ++tileCounter;
        }
    }

    if ((bestDistance == 0) || (bestDistance < minTilesDistance)) 
        return false;

    // randomize distance
    bestDistance = Random::GenerateInt(minTilesDistance, bestDistance);

    MapTile* destinationTile = sourceTile;
    for (int i = 0; i < bestDistance; ++i) 
    {
        destinationTile = destinationTile->mNeighbours[bestDistanceDirection];
    }

    resultPoint = MapUtils::ComputeTileCenter2d(destinationTile->mTileLocation);
    return true;
}
