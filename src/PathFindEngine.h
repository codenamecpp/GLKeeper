#pragma once

//////////////////////////////////////////////////////////////////////////

#include "NavigationDefs.h"

//////////////////////////////////////////////////////////////////////////

class PathFindEngine final: public cxx::noncopyable
{
private:
    
    //////////////////////////////////////////////////////////////////////////

    class PathFindNode;

    static constexpr float LargeCost = 100000000.0f;

    //////////////////////////////////////////////////////////////////////////

    class FringeList
    {
    public:
        PathFindNode* Remove(PathFindNode* node);
        void InsertAfter(PathFindNode* node, PathFindNode* prevNode);
        void InsertBefore(PathFindNode* node, PathFindNode* nextNode);
        inline void PushFront(PathFindNode* node) { InsertBefore(node, mListHead); }
        inline void PushBack (PathFindNode* node) { InsertAfter(node, mListTail); }
        inline void PopFront () { Remove(mListHead); }
        inline void PopBack  () { Remove(mListTail); }
        void Clear();
        inline PathFindNode* GetFirst() const { return mListHead; }
        inline PathFindNode* GetLast() const { return mListTail; }
        bool Contains(PathFindNode* node) const;
        inline bool Empty() const { return mListHead == nullptr; }
    private:
        PathFindNode* mListHead = nullptr;
        PathFindNode* mListTail = nullptr;
    };

    //////////////////////////////////////////////////////////////////////////

    class PathFindNode
    {
    public:
        PathFindNode() = default;
        inline void UpdateCostG(float costG, PathFindNode* parent)
        {
            mCostG = costG;
            mParentNode = parent;
        }
        inline float GetCostF() const { return mCostG + mCostH; }
        inline void SetOpened(unsigned int problemId)
        {
            mProblemId = problemId;
        }
        inline bool IsOpened(unsigned int problemId) const { return mProblemId == problemId; }
        inline PathFindNode* GetListNext() const { return mListNext; }
        inline PathFindNode* GetListPrev() const { return mListPrev; }
    public:
        MapPoint2D mCoord {};
        MapTile* mMapTile = nullptr;
        PathFindNode* mParentNode = nullptr;
        PathFindNode* mListNext = nullptr;
        PathFindNode* mListPrev = nullptr;
        FringeList* mList = nullptr;
        float mCostH = 0.0f;
        float mCostG = 0.0f;
        unsigned int mProblemId = 0;
    };

    //////////////////////////////////////////////////////////////////////////

    inline float Heuristic(const MapPoint2D& src, const MapPoint2D& dst) const
    {
        // octile
        float dx = float(std::abs(src.x - dst.x));
        float dy = float(std::abs(src.y - dst.y));
        constexpr float d  = 1.0f;
        constexpr float d2 = 1.4f;
        return d * (dx + dy) + (d2 - 2.0f * d) * std::min(dx, dy);
    }

    //////////////////////////////////////////////////////////////////////////

public:
    PathFindEngine();

    void EnterWorld();
    void ClearWorld();

    bool StartPathFind(const MapPoint2D& srcCoord, const MapPoint2D& dstCoord, ePassabilityType passabilityType, unsigned int cyclesLimit);
    void ClearPathFind();
    bool RunPathFind(unsigned int cyclesCount, unsigned int& cyclesTaken);

    // check whether pathfind in progress
    bool IsSearching() const;
    
    // get the result path for last operation
    // may be partial, in such case the final step is not the destination point
    inline const auto& GetResultPath() const { return mResultPath; }
    inline bool HasReresultPath() const 
    { 
        return !mResultPath.empty();
    }
    const MapPoint2D& GetSrcCoord() const { return mSrcCoord; }
    const MapPoint2D& GetDstCoord() const { return mDstCoord; }

protected:
    bool CheckWithinBounds(const MapPoint2D& coord) const;
    bool CalcStepCostG(PathFindNode* sourceNode, eDirection moveDirection, float& costG) const;
    void ResetNodesProblemId();
    void RunPathFindCycle();
    void FinishPathFind(PathFindNode* bestFinishNode);

    PathFindNode* GetPathFindNode(const MapPoint2D& coord);

private:
    MapPoint2D mMapDims; // chache
    FringeList mFringeList {};
    MapPoint2D mSrcCoord {}; // where from
    MapPoint2D mDstCoord {}; // where to
    PathFindNode* mLastBestNode = nullptr;
    PathFindNode* mExpandNode = nullptr;
    ePassabilityType mPassabilityType = ePassabilityType_Land;
    float mCostLimit = 0.0f;
    float mNextCostLimit = 0.0f;
    unsigned int mCyclesCounter = 0;
    unsigned int mCyclesLimit = 0;
    unsigned int mCurrentProblemId = 0;
    std::unique_ptr<PathFindNode[]> mNodes;
    std::vector<MapPoint2D> mResultPath;
};

//////////////////////////////////////////////////////////////////////////
