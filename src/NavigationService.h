#pragma once

//////////////////////////////////////////////////////////////////////////

#include "NavigationDefs.h"
#include "GameSessionAware.h"

//////////////////////////////////////////////////////////////////////////

class NavigationService final: private GameSessionAware
{
public:
    void EnterWorld();
    void ClearWorld();
    void UpdateFrame(float deltaTime);
    void UpdateLogic(float stepDeltaTime);

    bool GetRandomWanderingPointLocation(const MapPoint2D& srcTileLocation, 
        int minTilesDistance, 
        int maxTilesDistance, glm::vec2& resultPoint) const;

private:

};

//////////////////////////////////////////////////////////////////////////