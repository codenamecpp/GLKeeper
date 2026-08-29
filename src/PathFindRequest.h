#pragma once

//////////////////////////////////////////////////////////////////////////

#include "NavigationDefs.h"

//////////////////////////////////////////////////////////////////////////

class PathFindRequest final: public cxx::noncopyable
{
    friend class NavigationService;

public:
    PathFindRequest() = default;

    inline ePathFindStatus GetStatus() const { return mStatus; }
    // check whether pathfind task is not completed yet
    inline bool IsSearching() const 
    { 
        return !IsCompleted(); 
    }
    inline bool IsCompleted() const { return mStatus == ePathFindStatus_Completed; }
    // check whether found path is partial
    inline bool HasPartialPath() const
    {
        return (mStatus == ePathFindStatus_Completed) && !mPathTiles.empty() && (mPathTiles.back() == mDstTile);
    }
    inline bool HasFullPath() const
    {
        return (mStatus == ePathFindStatus_Completed) && !mPathTiles.empty() && (mPathTiles.back() == mDstTile);
    }
    inline bool HasPath() const
    {
        return !mPathWaypoints.empty();
    }

    inline const Point2D& GetSrcTile() const { return mSrcTile; }
    inline const Point2D& GetDstTile() const { return mDstTile; }

    inline const glm::vec2& GetSrcPosition() const { return mSrcPos; }
    inline const glm::vec2& GetDstPosition() const { return mDstPos; }

    inline ePassabilityType GetPassabilityType() const { return mPassabilityType; }

    // accessing result path
    inline const auto& GetResultPathTiles() const { return mPathTiles; }
    inline const auto& GetResultPathWaypoints() const { return mPathWaypoints; }

    inline int GetPathWaypointsCount() const
    {
        return static_cast<int>(mPathWaypoints.size());
    }

private:
    void Configure(const glm::vec2& srcPos, const glm::vec2& dstPos, ePassabilityType passabilityType);
    // pool
    void OnRecycle();

    void SetStatus(ePathFindStatus status);
    void SetResultPathFromTiles(cxx::span<const Point2D> pathTiles);

private:
    ePathFindStatus mStatus = ePathFindStatus_Pending;
    ePassabilityType mPassabilityType = ePassabilityType_Land;
    glm::vec2 mSrcPos {};
    glm::vec2 mDstPos {};
    Point2D mSrcTile {};
    Point2D mDstTile {};
    std::vector<Point2D> mPathTiles; // for debug
    std::vector<glm::vec2> mPathWaypoints;
};

//////////////////////////////////////////////////////////////////////////