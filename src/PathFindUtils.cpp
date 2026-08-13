#include "stdafx.h"
#include "PathFindUtils.h"
#include "MapTile.h"

bool PathFindUtils::CheckPassability(const MapTile* startTile, const MapTile* checkTile, ePassabilityType passabilityType)
{
    if ((startTile == nullptr) || (checkTile == nullptr))
    {
        cxx_assert(false);
        return false;
    }

    TerrainDefinition* terrainDef = checkTile->GetTerrain();
    if (terrainDef->mIsSolid) return false;
    if (terrainDef->mIsLava)
    {
        bool canWalkOnLava = (passabilityType == ePassabilityType_Land_Any);
        if (!canWalkOnLava) return false;
    }
    if (terrainDef->mIsWater)
    {
        bool canWalkOnWater = (passabilityType == ePassabilityType_Land_Any) || (passabilityType == ePassabilityType_Land_Water);
        if (!canWalkOnWater) return false;
    }
    return true;
}
