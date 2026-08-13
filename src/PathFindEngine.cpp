#include "stdafx.h"
#include "PathFindEngine.h"
#include "GameWorld.h"
#include "PathFindUtils.h"
#include "GameMap.h"

//////////////////////////////////////////////////////////////////////////

PathFindEngine::PathFindNode* PathFindEngine::FringeList::Remove(PathFindNode* node)
{
    if (!Contains(node)) 
        return nullptr;

    if (PathFindNode* prevNode = node->mListPrev)
    {
        prevNode->mListNext = node->mListNext;
    }
    else
    {
        mListHead = node->mListNext;
    }

    PathFindNode* nextNode = node->mListNext;
    if (nextNode)
    {
        nextNode->mListPrev = node->mListPrev;
    }
    else
    {
        mListTail = node->mListPrev;
    }
    node->mListNext = nullptr;
    node->mListPrev = nullptr;
    node->mList = nullptr;
    return nextNode;
}

void PathFindEngine::FringeList::InsertAfter(PathFindNode* node, PathFindNode* prevNode)
{
    // node must not be in any list
    if ((node == nullptr) || (node->mList != nullptr)) 
    {
        cxx_assert(false);
        return;
    }

    if (prevNode && !Contains(prevNode)) 
    {
        cxx_assert(false);
        return;
    }

    if (Empty())
    {
        cxx_assert(prevNode == nullptr);
        mListTail = node;
        mListHead = node;
        node->mList = this;
        return;
    }

    if (prevNode == nullptr)
    {
        prevNode = mListTail;
    }

    node->mListPrev = prevNode;

    if (PathFindNode* nextNode = prevNode->mListNext)
    {
        node->mListNext = nextNode;
        nextNode->mListPrev = node;
    }
    else
    {
        mListTail = node;
    }

    prevNode->mListNext = node;
    node->mList = this;
}

void PathFindEngine::FringeList::InsertBefore(PathFindNode* node, PathFindNode* nextNode)
{
    // node must nut be in any list
    if ((node == nullptr) || (node->mList != nullptr))
    {
        cxx_assert(false);
        return;
    }

    if (nextNode && !Contains(nextNode)) 
    {
        cxx_assert(false);
        return;
    }

    if (Empty())
    {
        cxx_assert(nextNode == nullptr);
        mListHead = node;
        mListTail = node;
        node->mList = this;
        return;
    }

    if (nextNode == nullptr)
    {
        nextNode = mListHead;
    }

    node->mListNext = nextNode;

    if (PathFindNode* prevNode = nextNode->mListPrev)
    {
        node->mListPrev = prevNode;
        prevNode->mListNext = node;
    }
    else
    {
        mListHead = node;
    }

    nextNode->mListPrev = node;
    node->mList = this;
}

void PathFindEngine::FringeList::Clear()
{
    while (!Empty())
    {
        PopBack();
    }
}

bool PathFindEngine::FringeList::Contains(PathFindNode* node) const
{
    return node && (node->mList == this);
}

//////////////////////////////////////////////////////////////////////////

PathFindEngine::PathFindEngine()
{
}

void PathFindEngine::EnterWorld()
{
    const GameMap& gameMap = gGameMap;

    mMapDims = gameMap.GetDimensions();

    cxx_assert(mMapDims.x > 0);
    cxx_assert(mMapDims.y > 0);

    // init nodes
    mNodes = std::move(std::unique_ptr<PathFindNode[]>(new PathFindNode[mMapDims.x * mMapDims.y]));
    
    for (int y = 0; y < mMapDims.y; ++y)
    {
        for (int x = 0; x < mMapDims.x; ++x)
        {
            PathFindNode& roller = mNodes[y * mMapDims.x + x];
            roller.mParentNode = nullptr;
            roller.mCoord = {x, y};
            roller.mMapTile = gameMap.GetMapTile(roller.mCoord);
            roller.mProblemId = 0;
        }
    }

    mCurrentProblemId = 0;
}

void PathFindEngine::ClearWorld()
{
    ClearPathFind();
    
    mMapDims = {};
    mNodes.reset();
}

bool PathFindEngine::IsSearching() const
{
    return !mFringeList.Empty();
}

bool PathFindEngine::StartPathFind(const MapPoint2D& srcCoord, const MapPoint2D& dstCoord, ePassabilityType passabilityType, unsigned int cyclesLimit)
{
    ClearPathFind();

    mSrcCoord = srcCoord;
    mDstCoord = dstCoord;

    cxx_assert(CheckWithinBounds(mSrcCoord));
    cxx_assert(CheckWithinBounds(mDstCoord));

    mPassabilityType = passabilityType;
    mCyclesLimit = cyclesLimit;
    
    cxx_assert(mPassabilityType < ePassabilityType_COUNT);
    
    if (++mCurrentProblemId == 0)
    {
        mCurrentProblemId = 1;
        // force reset all the nodes
        ResetNodesProblemId();
    }

    // initial node
    PathFindNode* startNode = GetPathFindNode(srcCoord);
    cxx_assert(startNode);
    startNode->SetOpened(mCurrentProblemId);

    mCostLimit = startNode->GetCostF();
    mNextCostLimit = LargeCost;
    mExpandNode = startNode;

    mFringeList.Clear();
    mFringeList.PushBack(startNode);

    return true;
}

void PathFindEngine::ClearPathFind()
{
    mFringeList.Clear();
    mSrcCoord = {};
    mDstCoord = {};
    mExpandNode = nullptr;
    mLastBestNode = nullptr;
    mPassabilityType = {};
    mCostLimit = 0.0f;
    mNextCostLimit = 0.0f;
    mCyclesCounter = 0;
    mCyclesLimit = 0;
    mResultPath.clear();
}

bool PathFindEngine::RunPathFind(unsigned int cyclesCount, unsigned int& cyclesTaken)
{
    cyclesTaken = 0;

    if (!IsSearching())
        return false;

    for ( ; cyclesTaken < cyclesCount; ++cyclesTaken)
    {
        RunPathFindCycle();
        if (!IsSearching())
            break;

        ++mCyclesCounter;
        // check cycles limit if specified
        if ((mCyclesLimit > 0) && (mCyclesCounter >= mCyclesLimit))
        {
            FinishPathFind(mLastBestNode);
        }
    }
    return true;
}

void PathFindEngine::RunPathFindCycle()
{
    cxx_assert(!mFringeList.Empty());

    PathFindNode* currentNode = mExpandNode;

    // select next node within the cost limit
    for (;;)
    {
        if (currentNode == nullptr)
        {
            // restart with new cost limit
            if (mCostLimit == mNextCostLimit)
            {
                FinishPathFind(mLastBestNode);
                return; // cannot exceed limit any further
            }
            mCostLimit = mNextCostLimit;
            mNextCostLimit = LargeCost;
            currentNode = mFringeList.GetFirst();
            cxx_assert(currentNode);
        }

        if (currentNode->GetCostF() > mCostLimit)
        {
            mNextCostLimit = std::min(mNextCostLimit, currentNode->GetCostF());
            currentNode = currentNode->GetListNext();
        }
        else break;
    }

    // store closest node to destination coord
    if ((mLastBestNode == nullptr) || 
        (mLastBestNode->mCostH > currentNode->mCostH))
    {
        mLastBestNode = currentNode;
    }

    // reached the goal?
    if (currentNode->mCoord == mDstCoord)
    {
        FinishPathFind(currentNode);
        return;
    }

    // expand
    for (eDirection dir: gDirectionsCW)
    {
        float stepCostG;
        if (!CalcStepCostG(currentNode, dir, stepCostG))
            continue;

        const float neighCostG = currentNode->mCostG + stepCostG;

        PathFindNode* neighNode = GetPathFindNode(currentNode->mCoord + gDirectionVectors[dir]);
        if (neighNode == nullptr) continue;
        if (!neighNode->IsOpened(mCurrentProblemId))
        {
            neighNode->SetOpened(mCurrentProblemId);
            neighNode->UpdateCostG(neighCostG, currentNode);
            mFringeList.InsertAfter(neighNode, currentNode);
            continue;
        }

        if (neighCostG < neighNode->mCostG)
        {
            neighNode->UpdateCostG(neighCostG, currentNode);
            mFringeList.Remove(neighNode);
            mFringeList.InsertAfter(neighNode, currentNode);
            continue;
        }
    }

    mExpandNode = mFringeList.Remove(currentNode);

    // nothing left to explore
    if ((mExpandNode == nullptr) && mFringeList.Empty())
    {
        FinishPathFind(mLastBestNode);
    }
}

void PathFindEngine::ResetNodesProblemId()
{
    for (int inode = 0, NumNodes = (mMapDims.x * mMapDims.y);
        inode < NumNodes; ++inode)
    {
        PathFindNode& roller = mNodes[inode];
        roller.mProblemId = 0;
    }
}

bool PathFindEngine::CheckWithinBounds(const MapPoint2D& coord) const
{
    if ((coord.x < 0) || (coord.y < 0)) 
        return false;

    return (coord.x < mMapDims.x) && (coord.y < mMapDims.y);
}

PathFindEngine::PathFindNode* PathFindEngine::GetPathFindNode(const MapPoint2D& coord)
{
    if (!CheckWithinBounds(coord))
        return nullptr;

    PathFindNode& pfnode = mNodes[coord.y * mMapDims.x + coord.x];
    if (!pfnode.IsOpened(mCurrentProblemId))
    {
        pfnode.mParentNode = nullptr;
        pfnode.mCostH = Heuristic(coord, mDstCoord);
    }
    return &pfnode;
}

bool PathFindEngine::CalcStepCostG(PathFindNode* sourceNode, eDirection moveDirection, float& costG) const
{
    MapTile* srcTile = sourceNode->mMapTile;
    MapTile* dstTile = srcTile->mNeighbours[moveDirection];

    if (!PathFindUtils::CheckPassability(srcTile, dstTile, mPassabilityType))
        return false;

    // diagonal movement
    bool isDiagonal = false;
    switch (moveDirection)
    {
        case eDirection_NE:
            isDiagonal = true;
            if (!PathFindUtils::CheckPassability(srcTile, srcTile->mNeighbours[eDirection_N], mPassabilityType) ||
                !PathFindUtils::CheckPassability(srcTile, srcTile->mNeighbours[eDirection_E], mPassabilityType))
            {
                return false;
            }
        break;
        case eDirection_SE:
            isDiagonal = true;
            if (!PathFindUtils::CheckPassability(srcTile, srcTile->mNeighbours[eDirection_S], mPassabilityType) ||
                !PathFindUtils::CheckPassability(srcTile, srcTile->mNeighbours[eDirection_E], mPassabilityType))
            {
                return false;
            }
        break;
        case eDirection_SW:
            isDiagonal = true;
            if (!PathFindUtils::CheckPassability(srcTile, srcTile->mNeighbours[eDirection_S], mPassabilityType) ||
                !PathFindUtils::CheckPassability(srcTile, srcTile->mNeighbours[eDirection_W], mPassabilityType))
            {
                return false;
            }
        break;
        case eDirection_NW:
            isDiagonal = true;
            if (!PathFindUtils::CheckPassability(srcTile, srcTile->mNeighbours[eDirection_N], mPassabilityType) ||
                !PathFindUtils::CheckPassability(srcTile, srcTile->mNeighbours[eDirection_W], mPassabilityType))
            {
                return false;
            }
        break;
    }

    costG = isDiagonal ? 1.4f : 1.0f;
    return true;
}

void PathFindEngine::FinishPathFind(PathFindNode* bestFinishNode)
{
    mResultPath.clear();
    mFringeList.Clear();
    while (bestFinishNode)
    {
        mResultPath.push_back(bestFinishNode->mCoord);
        bestFinishNode = bestFinishNode->mParentNode;
    }
    mExpandNode = nullptr;
    mLastBestNode = nullptr;
    if (!mResultPath.empty())
    {
        std::reverse(mResultPath.begin(), mResultPath.end());
    }
}
