#include "stdafx.h"
#include "PathFindRequest.h"
#include "MapUtils.h"

void PathFindRequest::OnRecycle()
{
    mStatus = {};
    mSrcPos = {};
    mDstPos = {};
    mSrcTile = {};
    mDstTile = {};
    mPassabilityType = {};
    mPathWaypoints.clear();
    mPathTiles.clear();
}

void PathFindRequest::Configure(const glm::vec2& srcPos, const glm::vec2& dstPos, ePassabilityType passabilityType)
{
    mSrcPos = srcPos;
    mDstPos = dstPos;
    mSrcTile = MapUtils::ComputeTileFromPosition(srcPos);
    mDstTile = MapUtils::ComputeTileFromPosition(dstPos);
    mPassabilityType = passabilityType;
}

void PathFindRequest::SetStatus(ePathFindStatus status)
{
    mStatus = status;
}

void PathFindRequest::SetResultPathFromTiles(cxx::span<const MapPoint2D> pathTiles)
{
    mPathWaypoints.clear();
    mPathTiles.clear();
    if (pathTiles.empty())
        return;

    mPathTiles.assign(pathTiles.begin(), pathTiles.end());

    // converting tile-based coords to world waypoints

    // always start off from src position
    mPathWaypoints.push_back(GetSrcPosition());

    // short path
    const int NumPathTiles = static_cast<int>(mPathTiles.size());
    if (NumPathTiles == 1)
    {
        if (mPathTiles.back() == GetDstTile())
        {
            mPathWaypoints.push_back(GetDstPosition());
        }
        return;
    }

    for (int i = 1; i < NumPathTiles - 1; ++i)
    {
        const MapPoint2D& currTile = mPathTiles[i];

        const MapPoint2D prevDelta = (currTile - mPathTiles[i - 1]);
        const MapPoint2D nextDelta = (mPathTiles[i + 1] - currTile);
        if (prevDelta != nextDelta)
        {
            // waypoint to tile center
            mPathWaypoints.push_back(MapUtils::ComputeTileCenter2d(currTile));
        }
    }

    // process last tile
    if (mPathTiles.back() == GetDstTile())
    {
        mPathWaypoints.push_back(GetDstPosition());
    }
    else
    {
        // waypoint to tile center
        mPathWaypoints.push_back(MapUtils::ComputeTileCenter2d(mPathTiles.back()));
    }
}